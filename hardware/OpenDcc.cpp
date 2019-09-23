#include <cstring>    //memset
#include <fcntl.h>
#include <sstream>
#include <termios.h>

#include "network/Select.h"
#include "hardware/OpenDcc.h"
#include "Utils/Utils.h"

namespace hardware
{

	// create instance of OpenDcc
	extern "C" OpenDcc* create_OpenDcc(const HardwareParams* params)
	{
		return new OpenDcc(params);
	}

	// delete instance of OpenDcc
	extern "C" void destroy_OpenDcc(OpenDcc* opendcc)
	{
		delete(opendcc);
	}

	OpenDcc::OpenDcc(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "OpenDCC / " + params->name + " at serial port " + params->arg1),
	 	logger(Logger::Logger::GetLogger("OpenDCC " + params->name + " " + params->arg1)),
	 	serialLine(logger, params->arg1, B19200, 8, 'N', 2),
		run(false)
	{
		logger->Info(name);

		s88Modules1 = Utils::Utils::StringToInteger(params->arg2, 0);
		s88Modules2 = Utils::Utils::StringToInteger(params->arg3, 0);
		s88Modules3 = Utils::Utils::StringToInteger(params->arg4, 0);
		s88Modules = s88Modules1 + s88Modules2 + s88Modules3;

		if (s88Modules > MaxS88Modules)
		{
			logger->Info("Too many S88 modules configured.");
			return;
		}

		if (s88Modules == 0)
		{
			logger->Info("No S88 modules configured.");
			return;
		}

		logger->Info("{0} ({1}/{2}/{3}) S88 modules configured.", s88Modules, s88Modules1, s88Modules2, s88Modules3);

		SendP50XOnly();
		bool ok = SendNop();
		if (!ok)
		{
			logger->Error("Control does not answer");
			return;
		}
		bool restart = false;
		unsigned char modules = SendXP88Get(0);
		if (modules != s88Modules)
		{
			logger->Info("Configuring {0} modules in total", s88Modules);
			SendXP88Set(0, s88Modules);
			restart = true;;
		}

		modules = SendXP88Get(1);
		if (modules != s88Modules1)
		{
			logger->Info("Configuring bus 1 with {0} modules", s88Modules1);
			SendXP88Set(1, s88Modules1);
			restart = true;;
		}

		modules = SendXP88Get(2);
		if (modules != s88Modules2)
		{
			logger->Info("Configuring bus 2 with {0} modules", s88Modules2);
			SendXP88Set(2, s88Modules2);
			restart = true;;
		}

		modules = SendXP88Get(3);
		if (modules != s88Modules3)
		{
			logger->Info("Configuring bus 3 with {0} modules", s88Modules3);
			SendXP88Set(3, s88Modules3);
			restart = true;;
		}

		if (restart)
		{
			SendXP88Set(4, 0);
			SendRestart();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			SendP50XOnly();
			ok = SendNop();
			if (!ok)
			{
				logger->Error("Control does not answer");
				return;
			}
		}

		checkEventsThread = std::thread(&hardware::OpenDcc::CheckEventsWorker, this);
	}

	OpenDcc::~OpenDcc()
	{
		if (!run)
		{
			return;
		}
		run = false;
		checkEventsThread.join();
	}

	void OpenDcc::Booster(const boosterState_t status)
	{
		if (!serialLine.IsConnected())
		{
			return;
		}

		if (status)
		{
			logger->Info("Turning booster on");
			SendPowerOn();
		}
		else
		{
			logger->Info("Turning booster off");
			SendPowerOff();
		}
	}

	void OpenDcc::LocoSpeed(__attribute__((unused)) const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed)
	{
		if (!serialLine.IsConnected() || !CheckLocoAddress(address))
		{
			return;
		}

		unsigned char speedOpenDcc;
		if (speed == 0)
		{
			speedOpenDcc = 0;
		}
		else if (speed > 1000)
		{
			speedOpenDcc = 127;
		}
		else
		{
			speedOpenDcc = speed >> 3;
			speedOpenDcc += 2;
		}

		unsigned char data[2];
		uint16_t* dataPointer = reinterpret_cast<uint16_t*>(data);
		*dataPointer = GetCacheBasicEntry(address);
		data[0] = speedOpenDcc;
		cacheBasic[address] = *dataPointer;

		SendXLok(address, data[0], data[1]);
	}


	void OpenDcc::LocoDirection(__attribute__((unused)) const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		if (!serialLine.IsConnected() || !CheckLocoAddress(address))
		{
			return;
		}

		unsigned char data[2];
		uint16_t* dataPointer = reinterpret_cast<uint16_t*>(data);
		*dataPointer = GetCacheBasicEntry(address);
		data[1] &= ~(1 << 5);
		data[1] |= static_cast<unsigned char>(direction) << 5;
		cacheBasic[address] = *dataPointer;

		SendXLok(address, data[0], data[1]);
	}

	void OpenDcc::LocoFunction(__attribute__((unused)) const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		if (!serialLine.IsConnected() || !CheckLocoAddress(address))
		{
			return;
		}

		if (function > MaxLocoFunctions)
		{
			return;
		}
		if (function == 0)
		{
			unsigned char data[2];
			uint16_t* dataPointer = reinterpret_cast<uint16_t*>(data);
			*dataPointer = GetCacheBasicEntry(address);
			data[1] &= ~(1 << 4);
			data[1] |= static_cast<unsigned char>(on) << 4;
			cacheBasic[address] = *dataPointer;
			SendXLok(address, data[0], data[1]);
			return;
		}

		uint32_t functions = GetCacheFunctionsEntry(address);
		unsigned char shift = function - 1;
		functions &= ~(1 << shift);
		functions |= static_cast<uint32_t>(on) << shift;
		cacheFunctions[address] = functions;
		unsigned char* functionsPointer = reinterpret_cast<unsigned char*>(&functions);

		if (function <= 8)
		{
			SendXFunc(address, functionsPointer[0]);
			return;
		}

		if (function <= 16)
		{
			SendXFunc2(address, functionsPointer[1]);
			return;
		}

		SendXFunc34(address, functionsPointer[2], functionsPointer[3]);
	}

	bool OpenDcc::SendXLok(const address_t address, const unsigned char speed, const unsigned char functions)
	{
		logger->Info("Setting speed of OpenDCC loco {0} to speed {1} and direction {2} and light {3}", address, speed, (functions >> 5) & 0x01, (functions >> 4) & 0x01);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[5] = { XLok, addressLSB, addressMSB, speed, functions };

		serialLine.Send(data, sizeof(data));
		char input;
		bool ret = serialLine.ReceiveExact(&input, 1);
		if (ret != 1)
		{
			logger->Warning("No answer to XLok command");
			return false;
		}
		switch (input)
		{
			case OK:
				return true;

			case XBADPRM:
				logger->Warning("XLok returned bad parameter");
				return false;

			case XLKHALT:
				logger->Warning("XLok returned OpenDCC on HALT");
				return false;

			case XLKPOFF:
				logger->Warning("XLok returned Power OFF");
				return false;

			default:
				logger->Warning("XLok returned unknown error code: {0}", static_cast<int>(input));
				return false;
		}
	}

	bool OpenDcc::SendXFunc(const address_t address, const unsigned char functions)
	{
		logger->Info("Setting functions 1-8 of OpenDCC loco {0} to {1}", address, functions);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[4] = { XFunc, addressLSB, addressMSB, functions };
		serialLine.Send(data, sizeof(data));
		return ReceiveFunctionCommandAnswer();
	}

	bool OpenDcc::SendXFunc2(const address_t address, const unsigned char functions)
	{
		logger->Info("Setting functions 9-16 of OpenDCC loco {0} to {1}", address, functions);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[4] = { XFunc2, addressLSB, addressMSB, functions };
		serialLine.Send(data, sizeof(data));
		return ReceiveFunctionCommandAnswer();
	}

	bool OpenDcc::SendXFunc34(const address_t address, const unsigned char functions3, const unsigned char functions4)
	{
		logger->Info("Setting functions 17-28 of OpenDCC loco {0} to {1} and {2}", address, functions3, functions4);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[5] = { XFunc34, addressLSB, addressMSB, functions3, functions4 };
		serialLine.Send(data, sizeof(data));
		return ReceiveFunctionCommandAnswer();
	}

	bool OpenDcc::ReceiveFunctionCommandAnswer()
	{
		char input;
		bool ret = serialLine.ReceiveExact(&input, 1);
		if (ret != 1)
		{
			logger->Warning("No answer to XFunc command");
			return false;
		}
		switch (input)
		{
			case OK:
				return true;

			case XBADPRM:
				logger->Warning("XFunc returned bad parameter");
				return false;

			case XNOSLOT:
				logger->Warning("XFunc returned queue full");
				return false;

			default:
				logger->Warning("XFunc returned unknown error code: {0}", static_cast<int>(input));
				return false;
		}
	}

	void OpenDcc::Accessory(__attribute__((unused)) const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		if (!serialLine.IsConnected() || !CheckAccessoryAddress(address))
		{
			return;
		}
		logger->Info("Setting OpenDCC accessory {0} to {1}/{2}", address, state, on);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char statusBits = (state << 7) | (on << 6);
		const unsigned char addressStatus = addressMSB | statusBits;
		const unsigned char data[3] = { XTrnt, addressLSB, addressStatus };
		serialLine.Send(data, sizeof(data));
		char input;
		bool ret = serialLine.ReceiveExact(&input, 1);
		if (ret != 1)
		{
			logger->Warning("No answer to XTrnt command");
			return;
		}
		switch (input)
		{
			case OK:
				return;

			case XBADPRM:
				logger->Warning("XTrnt returned bad parameter");
				return;

			case XLOWTSP:
				logger->Warning("XTrnt returned queue nearly full");
				return;

			default:
				logger->Warning("XTrnt returned unknown error code: {0}", static_cast<int>(input));
				return;
		}
	}

	bool OpenDcc::SendP50XOnly()
	{
		unsigned char data[6] = { 'X', 'Z', 'Z', 'A', '1', 0x0D };
		std::string output(reinterpret_cast<char*>(data), sizeof(data));
		serialLine.Send(data, sizeof(data));
		std::string input;
		serialLine.ReceiveExact(input, 34);
		return true;
	}

	bool OpenDcc::SendOneByteCommand(const unsigned char data)
	{
		serialLine.Send(data);
		char input[1];
		int ret = serialLine.Receive(input, sizeof(input));
		return ret > 0 && input[0] == OK;
	}

	bool OpenDcc::SendRestart()
	{
		unsigned char data[3] = { '@', '@', 0x0D };
		std::string output(reinterpret_cast<char*>(data), sizeof(data));
		logger->Info("Restarting OpenDCC");
		serialLine.Send(data, sizeof(data));
		return true;
	}

	unsigned char OpenDcc::SendXP88Get(unsigned char param)
	{
		unsigned char data[2] = { XP88Get, param };
		std::string output(reinterpret_cast<char*>(data), sizeof(data));
		serialLine.Send(data, sizeof(data));
		unsigned char input;
		size_t ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
		if (ret == 0 || input != OK)
		{
			return 0xFF;
		}
		ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
		if (ret == 0)
		{
			return 0xFF;
		}
		return input;
	}

	bool OpenDcc::SendXP88Set(unsigned char param, unsigned char value)
	{
		unsigned char data[3] = { XP88Set, param, value };
		std::string output(reinterpret_cast<char*>(data), sizeof(data));
		serialLine.Send(data, sizeof(data));
		unsigned char input;
		size_t ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
		if (ret == 0)
		{
			return false;
		}
		return (input == OK);
	}

	void OpenDcc::SendXEvent()
	{
		unsigned char data[1] = { XEvent };
		std::string output(reinterpret_cast<char*>(data), sizeof(data));
		serialLine.Send(data, sizeof(data));
		unsigned char input;
		size_t ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
		if (ret == 0)
		{
			return;
		}
		bool locoEvent = input & 0x01;
		bool sensorEvent = (input >> 1) & 0x01;
		bool powerEvent = (input >> 3) & 0x01;
		bool switchEvent = (input >> 5) & 0x01;

		while (true)
		{
			bool moreData = (input >> 7) & 0x01;
			if (!moreData)
			{
				break;
			}
			ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
			if (ret == 0)
			{
				break;
			}
		}

		if (sensorEvent)
		{
			logger->Debug("Sensor Event");
		}

		if (locoEvent)
		{
			logger->Debug("Loco Event");
		}

		if (powerEvent)
		{
			logger->Info("Power off detected");
			manager->Booster(ControlTypeHardware, BoosterStop);
		}

		if (switchEvent)
		{
			logger->Debug("Switch Event");
		}
	}

	void OpenDcc::CheckEventsWorker()
	{
		pthread_setname_np(pthread_self(), "OpenDcc");
		run = true;
		while (run)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			SendXEvent();
		}
	}
} // namespace

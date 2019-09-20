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

		s88Thread = std::thread(&hardware::OpenDcc::S88Worker, this);
	}

	OpenDcc::~OpenDcc()
	{
		if (!run)
		{
			return;
		}
		run = false;
		s88Thread.join();
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

		SendLocoSpeedDirection(address, data[0], data[1]);
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
		data[1] &= 0xDF;
		data[1] |= static_cast<unsigned char>(direction) << 5;
		cacheBasic[address] = *dataPointer;

		SendLocoSpeedDirection(address, data[0], data[1]);
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
			data[1] &= 0xEF;
			data[1] |= static_cast<unsigned char>(on) << 4;
			cacheBasic[address] = *dataPointer;

			SendLocoSpeedDirection(address, data[0], data[1]);
			return;
		}
	}

	bool OpenDcc::SendLocoSpeedDirection(const address_t& address, const unsigned char speed, const unsigned char functions)
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
			logger->Warning("No answer to locospeed command");
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

	/*
	void OpenDcc::Accessory(__attribute__((unused)) const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		if (!serialLine.IsConnected())
		{
			return;
		}
	}
	*/

	void OpenDcc::S88Worker()
	{
		pthread_setname_np(pthread_self(), "OpenDcc");
		run = true;
		while (run)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	bool OpenDcc::SendP50XOnly()
	{
		unsigned char data[6] = { 'X', 'Z', 'Z', 'A', '1', 0x0D };
		std::string output(reinterpret_cast<char*>(data), sizeof(data));
		logger->Info("Setting control to P50X only mode");
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
} // namespace

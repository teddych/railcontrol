/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#include <cstring>    //memset
#include <fcntl.h>
#include <sstream>
#include <termios.h>

#include "Network/Select.h"
#include "Hardware/OpenDcc.h"
#include "Utils/Utils.h"

namespace Hardware
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

		SendP50XOnly();
		bool ok = SendNop();
		if (!ok)
		{
			logger->Error("OpenDCC does not answer");
			return;
		}

		s88Modules1 = Utils::Utils::StringToInteger(params->arg2, 0);
		s88Modules2 = Utils::Utils::StringToInteger(params->arg3, 0);
		s88Modules3 = Utils::Utils::StringToInteger(params->arg4, 0);
		s88Modules = s88Modules1 + s88Modules2 + s88Modules3;

		if (s88Modules > MaxS88Modules)
		{
			logger->Error("Too many S88 modules configured.");
			return;
		}

		if (s88Modules == 0)
		{
			logger->Info("No S88 modules configured.");
			return;
		}

		logger->Info("{0} ({1}/{2}/{3}) S88 modules configured.", s88Modules, s88Modules1, s88Modules2, s88Modules3);
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

		checkEventsThread = std::thread(&Hardware::OpenDcc::CheckEventsWorker, this);
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

		cache.SetSpeed(address, speed);
		SendXLok(address);
	}


	void OpenDcc::LocoDirection(__attribute__((unused)) const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		if (!serialLine.IsConnected() || !CheckLocoAddress(address))
		{
			return;
		}

		cache.SetDirection(address, direction);
		SendXLok(address);
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
		cache.SetFunction(address, function, on);
		if (function == 0)
		{
			SendXLok(address);
			return;
		}

		if (function <= 8)
		{
			SendXFunc(address);
			return;
		}

		if (function <= 16)
		{
			SendXFunc2(address);
			return;
		}

		SendXFunc34(address);
	}

	bool OpenDcc::SendXLok(const address_t address) const
	{
		OpenDccCacheEntry entry = cache.GetData(address);
		logger->Info("Setting speed of OpenDCC loco {0} to speed {1} and direction {2} and light {3}", address, entry.speed, (entry.directionF0 >> 5) & 0x01, (entry.directionF0 >> 4) & 0x01);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[5] = { XLok, addressLSB, addressMSB, entry.speed, entry.directionF0 };

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

	bool OpenDcc::SendXFunc(const address_t address) const
	{
		OpenDccCacheEntry entry = cache.GetData(address);
		logger->Info("Setting functions 1-8 of OpenDCC loco {0} to {1}", address, entry.function[0]);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[4] = { XFunc, addressLSB, addressMSB, entry.function[0] };
		serialLine.Send(data, sizeof(data));
		return ReceiveFunctionCommandAnswer();
	}

	bool OpenDcc::SendXFunc2(const address_t address) const
	{
		OpenDccCacheEntry entry = cache.GetData(address);
		logger->Info("Setting functions 9-16 of OpenDCC loco {0} to {1}", address, entry.function[1]);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[4] = { XFunc2, addressLSB, addressMSB, entry.function[1] };
		serialLine.Send(data, sizeof(data));
		return ReceiveFunctionCommandAnswer();
	}

	bool OpenDcc::SendXFunc34(const address_t address) const
	{
		OpenDccCacheEntry entry = cache.GetData(address);
		logger->Info("Setting functions 17-28 of OpenDCC loco {0} to {1} and {2}", address, entry.function[2], entry.function[3]);
		const unsigned char addressLSB = (address & 0xFF);
		const unsigned char addressMSB = (address >> 8);
		const unsigned char data[5] = { XFunc34, addressLSB, addressMSB, entry.function[2], entry.function[3] };
		serialLine.Send(data, sizeof(data));
		return ReceiveFunctionCommandAnswer();
	}

	bool OpenDcc::ReceiveFunctionCommandAnswer() const
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

	bool OpenDcc::SendP50XOnly() const
	{
		unsigned char data[6] = { 'X', 'Z', 'Z', 'A', '1', 0x0D };
		serialLine.Send(data, sizeof(data));
		std::string input;
		serialLine.ReceiveExact(input, 34);
		return true;
	}

	bool OpenDcc::SendOneByteCommand(const unsigned char data) const
	{
		serialLine.Send(data);
		char input[1];
		int ret = serialLine.Receive(input, sizeof(input));
		return ret > 0 && input[0] == OK;
	}

	bool OpenDcc::SendRestart() const
	{
		unsigned char data[3] = { '@', '@', 0x0D };
		logger->Info("Restarting OpenDCC");
		serialLine.Send(data, sizeof(data));
		return true;
	}

	unsigned char OpenDcc::SendXP88Get(unsigned char param) const
	{
		unsigned char data[2] = { XP88Get, param };
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

	bool OpenDcc::SendXP88Set(unsigned char param, unsigned char value) const
	{
		unsigned char data[3] = { XP88Set, param, value };
		serialLine.Send(data, sizeof(data));
		unsigned char input;
		size_t ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
		if (ret == 0)
		{
			return false;
		}
		return (input == OK);
	}

	void OpenDcc::CheckSensorData(const unsigned char module, const unsigned char data) const
	{
		unsigned char diff = s88Memory[module] ^ data;
		s88Memory[module] = data;
		feedbackPin_t pinOverAll = module;
		pinOverAll <<= 3;
		for (unsigned char pinOnModule = 1; pinOnModule <= 8; ++pinOnModule)
		{
			if ((diff >> (8 - pinOnModule)) & 0x01)
			{
				DataModel::Feedback::feedbackState_t state = static_cast<DataModel::Feedback::feedbackState_t>((data >> (8 - pinOnModule)) & 0x01);
				logger->Info("state of pin {0} on module {1} is {2}", pinOnModule, module, state);
				manager->FeedbackState(controlID, pinOverAll + pinOnModule, state);
			}
		}
	}

	void OpenDcc::SendXEvtSen() const
	{
		unsigned char data[1] = { XEvtSen };
		serialLine.Send(data, sizeof(data));
		while (true)
		{
			unsigned char module;
			size_t ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&module), 1);
			if (ret == 0 || module == 0)
			{
				return;
			}

			--module;
			module <<= 1;

			unsigned char data[2];
			ret = serialLine.ReceiveExact(reinterpret_cast<char*>(data), sizeof(data));
			if (ret == 0)
			{
				return;
			}

			if (s88Memory[module] != data[0])
			{
				CheckSensorData(module, data[0]);
			}

			++module;

			if (s88Memory[module] != data[1])
			{
				CheckSensorData(module, data[1]);
			}
		}
	}

	void OpenDcc::SendXEvent() const
	{
		unsigned char data[1] = { XEvent };
		serialLine.Send(data, sizeof(data));
		unsigned char input;
		size_t ret = serialLine.ReceiveExact(reinterpret_cast<char*>(&input), 1);
		if (ret == 0)
		{
			return;
		}
		bool locoEvent = input & 0x01;
		bool sensorEvent = (input >> 2) & 0x01;
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
			SendXEvtSen();
		}

		if (locoEvent)
		{
			logger->Debug("Loco Event detected");
		}

		if (powerEvent)
		{
			manager->Booster(ControlTypeHardware, BoosterStop);
		}

		if (switchEvent)
		{
			logger->Debug("Switch Event detected");
		}
	}

	void OpenDcc::CheckEventsWorker()
	{
		Utils::Utils::SetThreadName("OpenDcc");
		run = true;
		while (run)
		{
			SendXEvent();
			std::this_thread::yield();
		}
	}
} // namespace

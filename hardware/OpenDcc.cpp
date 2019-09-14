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
		run(true)
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

		s88Thread = std::thread(&hardware::OpenDcc::S88Worker, this);
	}

	OpenDcc::~OpenDcc()
	{
		run = false;
		if (s88Modules > 0 && s88Modules <= MaxS88Modules)
		{
			s88Thread.join();
		}
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
			SendBoosterOn();
		}
		else
		{
			logger->Info("Turning booster off");
			SendBoosterOff();
		}
	}

	/*
	void OpenDcc::LocoSpeed(__attribute__((unused)) const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed)
	{
		if (!serialLine.IsConnected())
		{
			return;
		}
	}

	void OpenDcc::LocoDirection(__attribute__((unused)) const protocol_t& protocol, const address_t& address, __attribute__((unused)) const direction_t& direction)
	{
		if (!serialLine.IsConnected())
		{
			return;
		}
	}

	void OpenDcc::LocoFunction(__attribute__((unused)) const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		if (function > MaxLocoFunctions)
		{
			return;
		}

		if (!serialLine.IsConnected())
		{
			return;
		}

	}

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
		while (run)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	void OpenDcc::SendP50XOnly()
	{
		unsigned char data[5] = { 'X', 'Z', 'Z', 'V', '1' };
		serialLine.Send(data, sizeof(data));
	}

} // namespace

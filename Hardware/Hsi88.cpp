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

#include "Languages.h"
#include "Network/Select.h"
#include "Hardware/Hsi88.h"
#include "Utils/Utils.h"

namespace Hardware
{

	// create instance of Hsi88
	extern "C" Hsi88* create_Hsi88(const HardwareParams* params)
	{
		return new Hsi88(params);
	}

	// delete instance of Hsi88
	extern "C" void destroy_Hsi88(Hsi88* hsi88)
	{
		delete(hsi88);
	}

	Hsi88::Hsi88(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "HSI-88 / " + params->name + " at serial port " + params->arg1),
	 	logger(Logger::Logger::GetLogger("HSI-88 " + params->name + " " + params->arg1)),
	 	serialLine(logger, params->arg1, B9600, 8, 'N', 1),
		run(false)
	{
		logger->Info(name);

		memset(s88Memory, 0, sizeof(s88Memory));

		s88Modules1 = Utils::Utils::StringToInteger(params->arg2, 0);
		s88Modules2 = Utils::Utils::StringToInteger(params->arg3, 0);
		s88Modules3 = Utils::Utils::StringToInteger(params->arg4, 0);
		s88Modules = s88Modules1 + s88Modules2 + s88Modules3;

		if (s88Modules > MaxS88Modules)
		{
			logger->Error(Languages::TextTooManyS88Modules, s88Modules, MaxS88Modules);
			return;
		}

		if (s88Modules == 0)
		{
			logger->Error(Languages::TextNoS88Modules);
			return;
		}

		std::string version = GetVersion();
		logger->Info("HSI-88 version: {0}", version);

		unsigned char modulesConfigured = ConfigureS88();
		if (s88Modules != modulesConfigured)
		{
			logger->Error(Languages::TextHsi88ErrorConfiguring, modulesConfigured);
		}

		logger->Info(Languages::TextHsi88Configured, s88Modules, s88Modules1, s88Modules2, s88Modules3);

		checkEventsThread = std::thread(&Hardware::Hsi88::CheckEventsWorker, this);
	}

	Hsi88::~Hsi88()
	{
		if (!run)
		{
			return;
		}
		run = false;
		checkEventsThread.join();
	}

	std::string Hsi88::ReadUntilCR()
	{
		std::string data;
		char input = 0;
		while (true)
		{
			int ret = serialLine.Receive(&input,sizeof(input));
			if (ret < 0 || input == '\r')
			{
				return data;
			}
			data.append(&input, ret);
		}
	}

	std::string Hsi88::GetVersion()
	{
		const unsigned char command[2] = { 'v', '\r' };
		serialLine.Send(command, sizeof(command));
		return ReadUntilCR();
	}

	unsigned char Hsi88::ConfigureS88()
	{
		const unsigned char command [5] = { 's', static_cast<unsigned char>(s88Modules1 >> 1), static_cast<unsigned char>(s88Modules2 >> 1), static_cast<unsigned char>(s88Modules3 >> 1), '\r' };
		serialLine.Send(command, sizeof(command));
		char input[3];
		int ret = serialLine.Receive(input, sizeof(input));
		if (ret <= 0)
		{
			return 0;
		}
		if (input[0] != 's')
		{
			return 0;
		}
		return input[1] << 1;
	}

	void Hsi88::ReadData()
	{
		std::string data = ReadUntilCR();
		if (data.size() <= 0)
		{
			return;
		}
		if (data.size() <= 2)
		{
			return;
		}
		if (data[0] != 'i')
		{
			return;
		}
		unsigned char modules = data[1];
		for (unsigned char module = 0; module < modules; ++module)
		{
			unsigned char modulePosition = module * 3 + 2;
			unsigned char dataPosition = modulePosition + 1;
			unsigned char memoryPosition = (data[modulePosition] - 1) * 2;
			const unsigned char* dataByte = reinterpret_cast<const unsigned char*>(data.c_str()) + dataPosition;
			CheckFeedbackByte(dataByte + 1, s88Memory + memoryPosition, memoryPosition);
			CheckFeedbackByte(dataByte, s88Memory + memoryPosition + 1, memoryPosition + 1);
		}
	}

	void Hsi88::CheckFeedbackByte(const unsigned char* dataByte, unsigned char* memoryByte, const unsigned char module)
	{
		unsigned char diff = *memoryByte ^ *dataByte;
		if (!diff)
		{
			return;
		}
		*memoryByte = *dataByte;
		unsigned char pin = 0;
		while (diff)
		{
			if (diff & 0x01)
			{
				DataModel::Feedback::feedbackState_t state = static_cast<DataModel::Feedback::feedbackState_t>((*dataByte >> pin) & 0x01);
				logger->Info(Languages::TextFeedbackChange, pin + 1, module, state);
				manager->FeedbackState(controlID, module * 8 + pin + 1, state);
			}
			diff >>= 1;
			++pin;
		}
	}

	void Hsi88::CheckEventsWorker()
	{
		Utils::Utils::SetThreadName("HSI-88");
		run = true;
		while (run)
		{
			ReadData();
			std::this_thread::yield();
		}
	}
} // namespace

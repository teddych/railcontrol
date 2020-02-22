/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include "Hardware/Ecos.h"
#include "Utils/Utils.h"

using std::string;

namespace Hardware
{
	extern "C" Ecos* create_Ecos(const HardwareParams* params)
	{
		return new Ecos(params);
	}

	extern "C" void destroy_Ecos(Ecos* ecos)
	{
		delete(ecos);
	}

	Ecos::Ecos(const HardwareParams* params)
	:	HardwareInterface(params->GetManager(),
			params->GetControlID(),
			"ESU ECoS / " + params->GetName() + " at IP " + params->GetArg1()),
		logger(Logger::Logger::GetLogger("ECoS " + params->GetName() + " " + params->GetArg1())),
	 	run(false),
	 	tcp(logger, params->GetArg1(), EcosPort),
	 	readBufferPosition(0)
	{
		logger->Info(Languages::TextStarting, name);
		if (!tcp.IsConnected())
		{
			return;
		}
		receiverThread = std::thread(&Hardware::Ecos::Receiver, this);
	}

	Ecos::~Ecos()
	{
		if (run == false)
		{
			return;
		}
		run = false;
		receiverThread.join();
		logger->Info(Languages::TextTerminatingSenderSocket);
	}

	void Ecos::Booster(const boosterState_t status)
	{
		Send(status == BoosterGo ? "set(1,go)\n" : "set(1,stop)\n");
	}

//	void Ecos::LocoSpeed(const protocol_t protocol, const address_t address, const locoSpeed_t speed)
	void Ecos::LocoSpeed(__attribute__ ((unused)) const protocol_t protocol, __attribute__ ((unused)) const address_t address, __attribute__ ((unused)) const locoSpeed_t speed)
	{
	}

//	void Ecos::LocoDirection(const protocol_t protocol, const address_t address, const direction_t direction)
	void Ecos::LocoDirection(__attribute__ ((unused)) const protocol_t protocol, __attribute__ ((unused)) const address_t address, __attribute__ ((unused)) const direction_t direction)
	{
	}

//	void Ecos::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	void Ecos::LocoFunction(__attribute__ ((unused)) const protocol_t protocol, __attribute__ ((unused)) const address_t address, __attribute__ ((unused)) const function_t function, __attribute__ ((unused)) const bool on)
	{
	}

//	void Ecos::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	void Ecos::Accessory(__attribute__ ((unused)) const protocol_t protocol, __attribute__ ((unused)) const address_t address, __attribute__ ((unused)) const accessoryState_t state, __attribute__ ((unused)) const bool on)
	{
	}

	void Ecos::Send(const char* data)
	{
		int ret = tcp.Send(data);
		if (ret < 0)
		{
			logger->Error(Languages::TextUnableToSendDataToControl);
		}
	}

	void Ecos::ActivateBoosterUpdates()
	{
		Send("request(1,view)\n");
	}

	void Ecos::Receiver()
	{
		Utils::Utils::SetThreadName("ECoS");
		logger->Info(Languages::TextReceiverThreadStarted);

		run = true;
		while(run)
		{
			ssize_t datalen = tcp.Receive(readBuffer, sizeof(readBuffer));

			if (!run)
			{
				break;
			}

			if (datalen < 0)
			{
				if (errno == ETIMEDOUT)
				{
					continue;
				}

				logger->Error(Languages::TextUnableToReceiveData);
				break;
			}
			logger->Hex(reinterpret_cast<unsigned char*>(readBuffer), datalen);

			Parser();
		}
		tcp.Terminate();
		logger->Info(Languages::TextTerminatingReceiverThread);
	}

	char Ecos::ReadChar()
	{
		return readBuffer[readBufferPosition++];
	}

	bool Ecos::CheckChar(char charToCheck)
	{
		return charToCheck == readBuffer[readBufferPosition++];
	}

	void Ecos::Parser()
	{
		if (CheckChar('<'))
		{
			logger->Error(Languages::TextInvalidDataReceived);
			return;
		}
		char type = ReadChar();
		if (type == 'R')
		{
			ParseReply();
			return;
		}
		if (type == 'E')
		{
			ParseEvent();
			return;
		}
		logger->Error(Languages::TextInvalidDataReceived);
	}

	void Ecos::ParseReply()
	{
	}

	void Ecos::ParseEvent()
	{
	}
} // namespace

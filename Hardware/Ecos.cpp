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
	 	tcp(Network::TcpClient::GetTcpClientConnection(logger, params->GetArg1(), EcosPort)),
	 	readBufferLength(0),
		readBufferPosition(0)
	{
		logger->Info(Languages::TextStarting, name);
		if (!tcp.IsConnected())
		{
			return;
		}
		receiverThread = std::thread(&Hardware::Ecos::Receiver, this);
		ActivateBoosterUpdates();
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

	void Ecos::Receiver()
	{
		Utils::Utils::SetThreadName("ECoS");
		logger->Info(Languages::TextReceiverThreadStarted);

		run = true;
		while(run)
		{
			ReadLine();

			if (!run)
			{
				break;
			}

			if (readBufferLength == 0)
			{
				if (errno == ETIMEDOUT)
				{
					continue;
				}

				logger->Error(Languages::TextUnableToReceiveData);
				break;
			}

			Parser();
		}
		tcp.Terminate();
		logger->Info(Languages::TextTerminatingReceiverThread);
	}

	void Ecos::ReadLine()
	{
		readBufferPosition = 0;
		readBufferLength = -1;
		while(true)
		{
			int dataLength = tcp.Receive(readBuffer + readBufferPosition, 1);
			if (dataLength != 1)
			{
				readBufferLength = 0;
				break;
			}
			if (readBuffer[readBufferPosition] == '\r')
			{
				continue;
			}
			if (readBuffer[readBufferPosition] == '\n')
			{
				readBufferLength = readBufferPosition;
				break;
			}
			++readBufferPosition;
			if (readBufferPosition >= MaxMessageSize)
			{
				break;
			}
		}
		readBufferPosition = 0;
		if (readBufferLength == 0)
		{
			return;
		}
		logger->Hex(reinterpret_cast<unsigned char*>(readBuffer), readBufferLength);
	}

	void Ecos::Parser()
	{
		if (!CheckChar('<'))
		{
			logger->Error(Languages::TextInvalidDataReceived);
			return;
		}
		char type = ReadAndConsumeChar();
		switch (type)
		{
			case 'R':
				ParseReply();
				return;

			case 'E':
				ParseEvent();
				return;

			default:
				logger->Error(Languages::TextInvalidDataReceived);
				return;
		}
	}

	void Ecos::ParseReply()
	{
		if (CompareAndConsume("EPLY", 4) == false)
		{
			logger->Error(Languages::TextInvalidDataReceived);
			return;
		}
		ReadLine();
		while(GetChar() != '<')
		{
			// ParseReplyLine()
			ReadLine();
		}
		ParseEndLine();
	}

	void Ecos::ParseEvent()
	{
		if (CompareAndConsume("VENT", 4) == false)
		{
			logger->Error(Languages::TextInvalidDataReceived);
			return;
		}
		ParseInt();
		if (CheckGraterThenAtLineEnd() == false)
		{
			return;
		}
		ReadLine();
		while(GetChar() != '<')
		{
			ParseEventLine();
			ReadLine();
		}
		ParseEndLine();
	}

	void Ecos::ParseEventLine()
	{
		int object = ParseInt();
		SkipWhiteSpace();
		if (object == 1)
		{
			if (Compare("status[GO]", 10))
			{
				manager->Booster(ControlTypeHardware, BoosterGo);
			}
			else if (Compare("status[STOP]", 12))
			{
				manager->Booster(ControlTypeHardware, BoosterStop);
			}
			return;
		}
		string event = ReadUntilLineEnd();
		logger->Hex(event);
	}

	void Ecos::ParseEndLine()
	{
		if (CompareAndConsume("<END", 4) == false)
		{
			logger->Error(Languages::TextInvalidDataReceived);
			return;
		}
		int i = ParseInt();
		if (i == 0)
		{
			return;
		}
		SkipWhiteSpace();
		string error = ReadUntilChar('>');
		logger->Error(Languages::TextControlReturnedError, error);
	}

	string Ecos::ReadUntilChar(const char c)
	{
		string out;
		while(GetChar() != c)
		{
			out.append(1, ReadAndConsumeChar());
		}
		return out;
	}

	string Ecos::ReadUntilLineEnd()
	{
		string out = ReadUntilChar('\n');
		return out;
	}

	bool Ecos::Compare(const char* reference, const size_t size) const
	{
		for (size_t index = 0; index < size; ++index)
		{
			if (GetChar(index) != reference[index])
			{
				return false;
			}
		}
		return true;
	}

	bool Ecos::CompareAndConsume(const char* reference, const size_t size)
	{
		for (size_t index = 0; index < size; ++index)
		{
			if (ReadAndConsumeChar() != reference[index])
			{
				return false;
			}
		}
		return true;
	}

	bool Ecos::SkipOptionalChar(const char charToSkip)
	{
		if (charToSkip == GetChar())
		{
			++readBufferPosition;
			return true;
		}
		return false;
	}

	bool Ecos::IsNumber() const
	{
		const char c = GetChar();
		return (c >= '0' && c <= '9');
	}

	int Ecos::ParseInt()
	{
		SkipWhiteSpace();
		int out = 0;
		while(IsNumber())
		{
			out *= 10;
			out += ReadAndConsumeChar() - '0';
		}
		return out;
	}
} // namespace

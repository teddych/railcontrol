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

#include <cstring>
#include <vector>

#include "Hardware/Ecos.h"
#include "Utils/Utils.h"

using std::string;
using std::strlen;
using std::to_string;
using std::vector;

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

	const char* const Ecos::CommandActivateBoosterUpdates = "request(1,view)\n";
	const char* const Ecos::CommandQueryLocos = "queryObjects(10,protocol,addr,name)\n";
	const char* const Ecos::CommandQueryAccessories = "queryObjects(11,protocol,addr,name1,name2,name3)\n";
	const char* const Ecos::CommandQueryFeedbacks = "queryObjects(26)\n";

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
		std::memset(feedbackMemory, 0, MaxFeedbackModules);
		receiverThread = std::thread(&Hardware::Ecos::Receiver, this);
		SendActivateBoosterUpdates();
		SendQueryLocos();
		SendQueryAccessories();
		SendQueryFeedbacks();
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

	void Ecos::LocoSpeed(const protocol_t protocol, const address_t address, const locoSpeed_t speed)
	{
		unsigned int locomotiveData = ProtocolAddressToData(protocol, address);
		if (dataToLoco.count(locomotiveData) != 1)
		{
			return;
		}
		unsigned int locomotiveId = dataToLoco[locomotiveData];
		SendGetHandle(locomotiveId);
		string command = "set(" + to_string(locomotiveId) + ",speed[" + to_string(speed >> 3) + "])\n";
		Send(command.c_str());
	}

	void Ecos::LocoDirection(const protocol_t protocol, const address_t address, const direction_t direction)
	{
		unsigned int locomotiveData = ProtocolAddressToData(protocol, address);
		if (dataToLoco.count(locomotiveData) != 1)
		{
			return;
		}
		unsigned int locomotiveId = dataToLoco[locomotiveData];
		SendGetHandle(locomotiveId);
		string command = "set(" + to_string(locomotiveId) + ",dir[" + (direction == DirectionRight ? "0" : "1") + "])\n";
		Send(command.c_str());
	}

	void Ecos::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		unsigned int locomotiveData = ProtocolAddressToData(protocol, address);
		if (dataToLoco.count(locomotiveData) != 1)
		{
			return;
		}
		unsigned int locomotiveId = dataToLoco[locomotiveData];
		SendGetHandle(locomotiveId);
		string command = "set(" + to_string(locomotiveId) + ",func[" + to_string(function) + "," + (on == true ? "1" : "0") + "])\n";
		Send(command.c_str());
	}

	void Ecos::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		if (on == false)
		{
			return;
		}
		unsigned int accessoryData = ProtocolAddressToData(protocol, address);
		if (dataToAccessory.count(accessoryData) != 1)
		{
			return;
		}
		unsigned int accessoryId = dataToAccessory[accessoryData];
		SendGetHandle(accessoryId);
		string command = "set(" + to_string(accessoryId) + ",state[" + (state ? "0" : "1") + "])\n";
		Send(command.c_str());
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
		if (!CheckAndConsumeChar('<'))
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
		SkipWhiteSpace();

		if (Compare(CommandQueryLocos, strlen(CommandQueryLocos) - 1))
		{
			ParseQueryLocos();
			return;
		}

		if (Compare(CommandQueryAccessories, strlen(CommandQueryAccessories) - 1))
		{
			ParseQueryAccessories();
			return;
		}

		if (Compare(CommandQueryFeedbacks, strlen(CommandQueryFeedbacks) - 1))
		{
			ParseQueryFeedbacks();
			return;
		}

		ReadLine();
		while(GetChar() != '<')
		{
			// ParseLine()
			ReadLine();
		}
		ParseEndLine();
	}

	void Ecos::ParseQueryLocos()
	{
		ReadLine();
		while(GetChar() != '<')
		{
			ParseLocoData();
			ReadLine();
		}
		ParseEndLine();
	}

	void Ecos::ParseQueryAccessories()
	{
		ReadLine();
		while(GetChar() != '<')
		{
			ParseAccessoryData();
			ReadLine();
		}
		ParseEndLine();
	}

	void Ecos::ParseQueryFeedbacks()
	{
		ReadLine();
		while(GetChar() != '<')
		{
			ParseFeedbackData();
			ReadLine();
		}
		ParseEndLine();
	}

	void Ecos::ParseLocoData()
	{
		int locomotiveId = ParseInt();
		string option;
		string stringProtocol;
		string name;
		int address;
		ParseOption(option, stringProtocol);
		ParseOptionInt(option, address);
		ParseOption(option, name);
		int protocol = ProtocolNone;
		if (stringProtocol.compare("MFX") == 0)
		{
			protocol = ProtocolMFX;
		}
		else if (stringProtocol.compare("MM14") == 0)
		{
			protocol = ProtocolMM1;
		}
		else if (stringProtocol.compare("MM27") == 0)
		{
			protocol = ProtocolMM2;
		}
		else if (stringProtocol.compare("DCC14") == 0)
		{
			protocol = ProtocolDCC14;
		}
		else if (stringProtocol.compare("DCC28") == 0)
		{
			protocol = ProtocolDCC28;
		}
		else if (stringProtocol.compare("DCC128") == 0)
		{
			protocol = ProtocolDCC128;
		}
		if (protocol == ProtocolNone)
		{
			logger->Warning(Languages::TextInvalidDataReceived);
			return;
		}
		unsigned int locomotiveData = ProtocolAddressToData(protocol, address);
		locoToData[locomotiveId] = locomotiveData;
		dataToLoco[locomotiveData] = locomotiveId;
		SendActivateUpdates(locomotiveId);

		logger->Info(Languages::TextFoundLocoInEcosDatabase, locomotiveId, protocol, address, name);
	}

	void Ecos::ParseAccessoryData()
	{
		int accessoryId = ParseInt();
		string option;
		string stringProtocol;
		string name1;
		string name2;
		string name3;
		int address;
		ParseOption(option, stringProtocol);
		ParseOptionInt(option, address);
		ParseOption(option, name1);
		ParseOption(option, name2);
		ParseOption(option, name3);
		int protocol = ProtocolNone;
		if (stringProtocol.compare("MOT") == 0)
		{
			protocol = ProtocolMM;
		}
		else if (stringProtocol.compare("DCC") == 0)
		{
			protocol = ProtocolDCC;
		}
		if (protocol == ProtocolNone)
		{
			logger->Warning(Languages::TextInvalidDataReceived);
			return;
		}
		unsigned int accessoryData = ProtocolAddressToData(protocol, address);
		accessoryToData[accessoryId] = accessoryData;
		dataToAccessory[accessoryData] = accessoryId;
		SendActivateUpdates(accessoryId);

		logger->Info(Languages::TextFoundAccessoryInEcosDatabase, accessoryId, protocol, address, name1, name2, name3);
	}

	void Ecos::ParseFeedbackData()
	{
		int feedbackId = ParseInt();
		SendActivateUpdates(feedbackId);
		logger->Info(Languages::TextFoundFeedbackModuleInEcosDatabase, feedbackId);
	}

	void Ecos::ParseOption(string& option, string& value)
	{
		SkipWhiteSpace();
		option = ReadUntilChar('[');
		if (!CheckAndConsumeChar('['))
		{
			return;
		}
		if (GetChar() == '"')
		{
			CheckAndConsumeChar('"');
			value = ReadUntilChar('"');
			return;
		}
		value = ReadUntilChar(']');
	}

	void Ecos::ParseOptionInt(string& option, int& value)
	{
		string stringValue;
		ParseOption(option, stringValue);
		value = Utils::Utils::StringToInteger(stringValue, 0);
	}

	void Ecos::ParseOptionHex(string& option, int& value)
	{
		string stringValue;
		ParseOption(option, stringValue);
		value = Utils::Utils::StringToInteger(stringValue, 0, true);
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
			logger->Error(Languages::TextInvalidDataReceived);
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

		if (object >= 20000)
		{
			ParseAccessoryEvent(object);
			return;
		}

		if (object >= 1000)
		{
			ParseLocoEvent(object);
			return;
		}

		if (object >= 100)
		{
			ParseFeedbackEvent(object);
			return;
		}

		if (object == 1)
		{
			ParseBoosterEvent();
			return;
		}

		string event = ReadUntilLineEnd();
		logger->Hex(event);
	}

	void Ecos::ParseBoosterEvent()
	{
		if (Compare("status[GO]", 10))
		{
			manager->Booster(ControlTypeHardware, BoosterGo);
		}
		else if (Compare("status[STOP]", 12))
		{
			manager->Booster(ControlTypeHardware, BoosterStop);
		}
	}

	void Ecos::ParseLocoEvent(int loco)
	{
		if (locoToData.count(loco) != 1)
		{
			return;
		}
		string option;
		string value;
		ParseOption(option, value);

		if (option.compare("speed") == 0)
		{
			address_t address;
			protocol_t protocol;
			GetLocoProtocolAddress(loco, protocol, address);
			locoSpeed_t speed = Utils::Utils::StringToInteger(value) << 3;
			manager->LocoSpeed(ControlTypeHardware, controlID, protocol, address, speed);
			return;
		}

		if (option.compare("dir") == 0)
		{
			address_t address;
			protocol_t protocol;
			GetLocoProtocolAddress(loco, protocol, address);
			direction_t direction = (Utils::Utils::StringToInteger(value) == 1 ? DirectionLeft : DirectionRight);
			manager->LocoDirection(ControlTypeHardware, controlID, protocol, address, direction);
			return;
		}

		if (option.compare("func") == 0)
		{
			address_t address;
			protocol_t protocol;
			GetLocoProtocolAddress(loco, protocol, address);
			vector<string> valueList;
			Utils::Utils::SplitString(value, ",", valueList);
			if (valueList.size() < 2)
			{
				logger->Error(Languages::TextInvalidDataReceived);
				return;
			}
			function_t function = Utils::Utils::StringToInteger(valueList[0], 0);
			bool on = Utils::Utils::StringToBool(valueList[1]);
			manager->LocoFunction(ControlTypeHardware, controlID, protocol, address, function, on);
			return;
		}
	}

	void Ecos::ParseAccessoryEvent(int accessory)
	{
		if (accessoryToData.count(accessory) != 1)
		{
			return;
		}
		string option;
		int value;
		ParseOptionInt(option, value);

		if (option.compare("state") == 0)
		{
			address_t address;
			protocol_t protocol;
			GetAccessoryProtocolAddress(accessory, protocol, address);
			accessoryState_t state = (value == 0);
			manager->AccessoryState(ControlTypeHardware, controlID, protocol, address, state);
			return;
		}
	}

	void Ecos::ParseFeedbackEvent(int feedback)
	{
		string option;
		int value;
		ParseOptionHex(option, value);

		if (option.compare("state") == 0)
		{
			unsigned int module1 = (feedback - 100) << 1;
			unsigned int module2 = module1 + 1;
			uint8_t moduleData1 = value & 0xFF;
			uint8_t moduleData2 = (value >> 8) & 0xFF;
			CheckFeedbackDiff(module1, moduleData1);
			CheckFeedbackDiff(module2, moduleData2);
			return;
		}
	}

	void Ecos::CheckFeedbackDiff(unsigned int module, uint8_t data)
	{
		uint8_t oldData = feedbackMemory[module];
		if (oldData == data)
		{
			return;
		}

		for(unsigned int pin = 0; pin < 8; ++pin)
		{
			uint8_t oldPinData = (oldData >> pin) & 0x01;
			uint8_t pinData = (data >> pin) & 0x01;
			if (oldPinData == pinData)
			{
				continue;
			}
			const unsigned int address = (module << 3) + pin + 1;
			const DataModel::Feedback::feedbackState_t state = pinData == 1 ? DataModel::Feedback::FeedbackStateOccupied : DataModel::Feedback::FeedbackStateFree;
			manager->FeedbackState(controlID, address, state);
			logger->Info(Languages::TextFeedbackChange, address & 0x000F, address >> 4, state);
		}
		feedbackMemory[module] = data;
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
		while(true)
		{
			const char readChar = GetChar();
			if (readChar == c || readChar == 0)
			{
				return out;
			}
			out.append(1, ReadAndConsumeChar());
		}
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

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

#pragma once

#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "Logger/Logger.h"
#include "Network/TcpClient.h"

namespace Hardware
{
	class Ecos : protected HardwareInterface
	{
		public:
			Ecos(const HardwareParams* params);
			~Ecos();

			bool CanHandleLocos() const override { return true; }
			bool CanHandleAccessories() const override { return true; }
			bool CanHandleFeedback() const override { return true; }

			void GetLocoProtocols(std::vector<protocol_t>& protocols) const override
			{
				protocols.push_back(ProtocolMM1);
				protocols.push_back(ProtocolMM2);
				protocols.push_back(ProtocolMFX);
				protocols.push_back(ProtocolDCC14);
				protocols.push_back(ProtocolDCC28);
				protocols.push_back(ProtocolDCC128);
				protocols.push_back(ProtocolSX1);
			}

			bool LocoProtocolSupported(protocol_t protocol) const override
			{
				return (protocol == ProtocolMM1
				|| protocol == ProtocolMM2
				|| protocol == ProtocolMFX
				|| protocol == ProtocolDCC14
				|| protocol == ProtocolDCC28
				|| protocol == ProtocolDCC128
				|| protocol == ProtocolSX1);
			}

			void GetAccessoryProtocols(std::vector<protocol_t>& protocols) const override
			{
				protocols.push_back(ProtocolMM);
				protocols.push_back(ProtocolDCC);
			}

			bool AccessoryProtocolSupported(protocol_t protocol) const override
			{
				return (protocol == ProtocolMM || protocol == ProtocolDCC);
			}

			static void GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes)
			{
				argumentTypes[1] = IpAddress;
			}

			void Booster(const boosterState_t status) override
			{
				Send(status == BoosterGo ? "set(1,go)\n" : "set(1,stop)\n");
			}

			void LocoSpeed(const protocol_t protocol, const address_t address, const locoSpeed_t speed) override;
			void LocoDirection(const protocol_t protocol, const address_t address, const direction_t direction) override;
			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

			static const char* const CommandActivateBoosterUpdates;
			static const char* const CommandQueryLocos;
			static const char* const CommandQueryAccessories;
			static const char* const CommandQueryFeedbacks;

		private:
			static const unsigned short MaxMessageSize = 1024;
			static const unsigned short EcosPort = 15471;

			void Send(const char* data);
			void Receiver();
			void ReadLine();
			void Parser();
			void ParseReply();
			void ParseQueryLocos();
			void ParseQueryAccessories();
			void ParseQueryFeedbacks();
			void ParseLocoData();
			void ParseAccessoryData();
			void ParseFeedbackData();

			void ParseEvent();
			void ParseEventLine();
			void ParseBoosterEvent();
			void ParseLocoEvent(int loco);
			void ParseAccessoryEvent(int accessory);
			void ParseFeedbackEvent(int feedback);
			void CheckFeedbackDiff(unsigned int module, uint8_t data);

			void ParseOption(std::string& option, std::string& value);
			void ParseOptionInt(std::string& option, int& value);
			void ParseOptionHex(std::string& option, int& value);
			void ParseEndLine();
			std::string ReadUntilChar(const char c);
			std::string ReadUntilLineEnd();

			void SendActivateBoosterUpdates()
			{
				Send(CommandActivateBoosterUpdates);
			}

			void SendQueryLocos()
			{
				Send(CommandQueryLocos);
			}

			void SendQueryAccessories()
			{
				Send(CommandQueryAccessories);
			}

			void SendQueryFeedbacks()
			{
				Send(CommandQueryFeedbacks);
			}

			void SendActivateUpdates(const int id)
			{
				std::string command = "request(" + std::to_string(id) + ",view)\n";
				Send(command.c_str());
			}

			void SendGetHandle(const int id)
			{
				std::string command = "request(" + std::to_string(id) + ",control,force)\n";
				Send(command.c_str());
			}

			char GetChar(const size_t offset = 0) const
			{
				size_t position = readBufferPosition + offset;
				if (position >= MaxMessageSize)
				{
					return 0;
				}
				return readBuffer[position];
			}

			char ReadAndConsumeChar()
			{
				if (readBufferPosition >= MaxMessageSize)
				{
					return 0;
				}
				return readBuffer[readBufferPosition++];
			}

			bool CheckAndConsumeChar(const char charToCheck)
			{
				if (readBufferPosition >= MaxMessageSize)
				{
					return false;
				}
				return charToCheck == readBuffer[readBufferPosition++];
			}

			bool CheckChar(const char charToCheck)
			{
				if (readBufferPosition >= MaxMessageSize)
				{
					return false;
				}
				return charToCheck == readBuffer[readBufferPosition];
			}

			bool SkipOptionalChar(const char charToSkip);

			void SkipWhiteSpace()
			{
				while(SkipOptionalChar(' '));
			}

			bool Compare(const char* reference, const size_t size) const;
			bool CompareAndConsume(const char* reference, const size_t size);
			bool IsNumber() const;
			int ParseInt();

			bool CheckGraterThenAtLineEnd()
			{
				SkipWhiteSpace();
				return CompareAndConsume(">\n", 2);
			}

			static unsigned int ProtocolAddressToData(int protocol, int address)
			{
				return (static_cast<unsigned int>(protocol) << 16) + static_cast<unsigned int>(address);
			}

			void GetLocoProtocolAddress(const unsigned int loco, protocol_t& protocol, address_t& address)
			{
				unsigned int locomotiveData = locoToData[loco];
				protocol = DataToProtocol(locomotiveData);
				address = DataToAddress(locomotiveData);
			}

			void GetAccessoryProtocolAddress(const unsigned int accessory, protocol_t& protocol, address_t& address)
			{
				unsigned int accessoryData = accessoryToData[accessory];
				protocol = DataToProtocol(accessoryData);
				address = DataToAddress(accessoryData);
			}

			static protocol_t DataToProtocol(unsigned int data)
			{
				return static_cast<protocol_t>(data >> 16);
			}

			static address_t DataToAddress(unsigned int data)
			{
				return data & 0xFFFF;
			}

			Logger::Logger* logger;
			volatile bool run;
			std::thread receiverThread;

			Network::TcpConnection tcp;

			char readBuffer[MaxMessageSize];
			ssize_t readBufferLength;
			size_t readBufferPosition;

			std::map<unsigned int,unsigned int> locoToData;
			std::map<unsigned int,unsigned int> dataToLoco;

			std::map<unsigned int,unsigned int> accessoryToData;
			std::map<unsigned int,unsigned int> dataToAccessory;

			static const unsigned int MaxFeedbackModules = 128;
			uint8_t feedbackMemory[MaxFeedbackModules];
	};

	extern "C" Ecos* create_Ecos(const HardwareParams* params);
	extern "C" void destroy_Ecos(Ecos* ecos);
} // namespace

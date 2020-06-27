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

#include "DataModel/AccessoryBase.h"
#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "Hardware/Capabilities.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"

// CAN protocol specification at http://streaming.maerklin.de/public-media/cs2/cs2CAN-Protokoll-2_0.pdf

namespace Hardware
{
	class ProtocolMaerklinCAN : protected HardwareInterface
	{
		public:
			ProtocolMaerklinCAN() = delete;

			inline Hardware::Capabilities GetCapabilities() const override
			{
				return Hardware::CapabilityLoco
					| Hardware::CapabilityAccessory
					| Hardware::CapabilityFeedback
					| Hardware::CapabilityProgram
					| Hardware::CapabilityProgramMmWrite
					| Hardware::CapabilityProgramMfxRead
					| Hardware::CapabilityProgramMfxWrite
					| Hardware::CapabilityProgramDccDirectRead
					| Hardware::CapabilityProgramDccDirectWrite
					| Hardware::CapabilityProgramDccPomWrite;
			}

			void GetLocoProtocols(std::vector<Protocol> &protocols) const override
			{
				protocols.push_back(ProtocolMM);
				protocols.push_back(ProtocolMFX);
				protocols.push_back(ProtocolDCC);
			}

			inline bool LocoProtocolSupported(Protocol protocol) const override
			{
				return (protocol == ProtocolMM || protocol == ProtocolMFX || protocol == ProtocolDCC);
			}

			inline void GetAccessoryProtocols(std::vector<Protocol> &protocols) const override
			{
				protocols.push_back(ProtocolMM);
				protocols.push_back(ProtocolDCC);
			}

			inline bool AccessoryProtocolSupported(Protocol protocol) const override
			{
				return (protocol == ProtocolMM || protocol == ProtocolDCC);
			}

			void Booster(const BoosterState status) override;
			void LocoSpeed(const Protocol protocol, const Address address, const Speed speed) override;
			void LocoOrientation(const Protocol protocol, const Address address, const Orientation orientation) override;
			void LocoFunction(const Protocol protocol, const Address address, const Function function, const DataModel::LocoFunctions::FunctionState on) override;
			void AccessoryOnOrOff(const Protocol protocol, const Address address, const DataModel::AccessoryState state, const bool on) override;
			void ProgramRead(const ProgramMode mode, const Address address, const CvNumber cv) override;
			void ProgramWrite(const ProgramMode mode, const Address address, const CvNumber cv, const CvValue value) override;

		protected:
			ProtocolMaerklinCAN(Manager* manager, ControlID controlID, Logger::Logger* logger, std::string name)
			:	HardwareInterface(manager, controlID, name),
			 	logger(logger),
				run(false),
			 	hasCs2Master(false)
			{
				GenerateUidHash();
			}

			void Init();

			virtual ~ProtocolMaerklinCAN();

			void Parse(const unsigned char* buffer);

			void Ping();

			virtual void Receiver() = 0;

			static const unsigned char CANCommandBufferLength = 13;

			Logger::Logger* logger;
			volatile bool run;

		private:
			enum CanCommand : unsigned char
			{
				CanCommandSystem = 0x00,
				CanCommandLocoSpeed = 0x04,
				CanCommandLocoDirection = 0x05,
				CanCommandLocoFunction = 0x06,
				CanCommandReadConfig = 0x07,
				CanCommandWriteConfig = 0x08,
				CanCommandAccessory = 0x0B,
				CanCommandS88Event = 0x11,
				CanCommandPing = 0x18,
				CanCommandRequestConfigData = 0x20,
				CanCommandConfigData = 0x21,
				CanCommandHello = 0x42
			};

			enum CanSubCommand : unsigned char
			{
				CanSubCommandStop = 0x00,
				CanSubCommandGo = 0x01
			};

			enum CanResponse : unsigned char
			{
				CanResponseCommand = 0x00,
				CanResponseResponse = 0x01
			};

			enum CanDeviceType : uint16_t
			{
				CanDeviceGfp = 0x0000,
				CanDeviceGleisbox = 0x0010,
				CanDeviceConnect6021 = 0x0020,
				CanDeviceMs2 = 0x0030,
				CanDeviceMs2_2 = 0x0032,
				CanDeviceWireless = 0xffe0,
				CanDeviceCs2Master = 0xffff
			};

			typedef unsigned char CanPrio;
			typedef unsigned char CanLength;
			typedef uint32_t CanAddress;
			typedef uint32_t CanUid;
			typedef uint16_t CanHash;

			void CreateCommandHeader(unsigned char* const buffer, const CanCommand command, const CanResponse response, const CanLength length);
			void ParseAddressProtocol(const unsigned char* const buffer, CanAddress& address, Protocol& protocol);

			inline CanPrio ParsePrio(const unsigned char* const buffer)
			{
				return buffer[0] >> 1;
			}

			inline CanCommand ParseCommand(const unsigned char* const buffer)
			{
				return static_cast<CanCommand>((CanCommand)(buffer[0]) << 7 | (CanCommand)(buffer[1]) >> 1);
			}

			inline CanSubCommand ParseSubCommand(const unsigned char* const buffer)
			{
				return static_cast<CanSubCommand>(buffer[9]);
			}

			inline CanResponse ParseResponse(const unsigned char* const buffer)
			{
				return static_cast<CanResponse>(buffer[1] & 0x01);
			}

			inline CanLength ParseLength(const unsigned char* const buffer)
			{
				return buffer[4];
			}

			inline CanAddress ParseAddress(const unsigned char* const buffer)
			{
				return Utils::Utils::DataBigEndianToInt(buffer + 5);
			}

			inline CanHash ParseHash(const unsigned char* const buffer)
			{
				return Utils::Utils::DataBigEndianToShort(buffer + 2);
			}

			inline CanUid ParseUid(const unsigned char* const buffer)
			{
				return Utils::Utils::DataBigEndianToInt(buffer + 5);
			}

			void ParsePingCommand(const unsigned char* const buffer);
			void ParsePingResponse(const unsigned char* const buffer);

			static CanHash CalcHash(const CanUid uid);
			void GenerateUidHash();

			void CreateLocalIDLoco(unsigned char* buffer, const Protocol& protocol, const Address& address);
			void CreateLocalIDAccessory(unsigned char* buffer, const Protocol& protocol, const Address& address);

			void Wait(const unsigned int duration) const;
			void Cs2MasterThread();

			inline void SendInternal(const unsigned char* buffer)
			{
				logger->Hex(buffer, 5 + ParseLength(buffer));
				Send(buffer);
			}
			virtual void Send(const unsigned char* buffer) = 0;

			CanUid uid;
			CanHash hash;
			bool hasCs2Master;
			std::thread receiverThread;
			std::thread cs2MasterThread;
	};
} // namespace


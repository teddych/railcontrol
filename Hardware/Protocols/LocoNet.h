/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2022 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include "Hardware/HardwareInterface.h"
#include "Hardware/HardwareParams.h"
#include "Logger/Logger.h"
#include "Network/Serial.h"

// protocol specification at https://www.digitrax.com/static/apps/cms/media/documents/loconet/loconetpersonaledition.pdf

namespace Hardware
{
	namespace Protocols
	{
		class LocoNet: Hardware::HardwareInterface
		{
			public:
				LocoNet() = delete;
				LocoNet(const LocoNet&) = delete;
				LocoNet& operator=(const LocoNet&) = delete;

				LocoNet(const HardwareParams* params,
					const std::string& controlName,
					const unsigned int dataSpeed);

				virtual ~LocoNet();

				inline Hardware::Capabilities GetCapabilities() const override
				{
					return Hardware::CapabilityAccessory;
				}

				void GetAccessoryProtocols(std::vector<Protocol>& protocols) const override
				{
					protocols.push_back(ProtocolServer);
				}

				bool AccessoryProtocolSupported(Protocol protocol) const override
				{
					return (protocol == ProtocolServer);
				}

				virtual void Booster(const BoosterState status) override;

				virtual void AccessoryOnOrOff(__attribute__((unused)) const Protocol protocol,
					const Address address,
					const DataModel::AccessoryState state,
					const bool on) override;

			private:
				enum OpCodes : unsigned char
				{
					OPC_BUSY         = 0x81,
					OPC_GPOFF        = 0x82,
					OPC_GPON         = 0x83,
					OPC_IDLE         = 0x85,
					OPC_LOCO_SPD     = 0xA0,
					OPC_LOCO_DIRF    = 0xA1,
					OPC_LOCO_SND     = 0xA2,
					OPC_LOCO_FUNC    = 0xA3,
					OPC_SW_REQ       = 0xB0,
					OPC_SW_REP       = 0xB1,
					OPC_INPUT_REP    = 0xB2,
					OPC_LONG_ACK     = 0xB4,
					OPC_SLOT_STAT1   = 0xB5,
					OPC_CONSIST_FUNC = 0xB6,
					OPC_UNLINK_SLOTS = 0xB8,
					OPC_LINK_SLOTS   = 0xB9,
					OPC_MOVE_SLOTS   = 0xBA,
					OPC_RQ_SL_DATA   = 0xBB,
					OPC_SW_STATE     = 0xBC,
					OPC_SW_ACK       = 0xBD,
					OPC_LOCO_ADR     = 0xBF,
					OPC_LOCO_FUNC2   = 0xD4,
					OPC_SL_RD_DATA   = 0xE7,
					OPC_WR_SL_DATA   = 0xEF
				};

				void Receiver();

				static void CalcCheckSum(unsigned char* data, const unsigned char length, unsigned char* checkSum);

				void Parse(unsigned char* data);

				volatile bool run;
				mutable Network::Serial serialLine;
				std::thread receiverThread;
		};
	} // namespace
} // namespace


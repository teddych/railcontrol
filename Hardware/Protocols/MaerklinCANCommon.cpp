/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

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
#include <ctime>
#include <deque>
#include <sstream>

#include "DataModel/LocoConfig.h"
#include "Hardware/Protocols/MaerklinCANCommon.h"
#include "Hardware/ZLib.h"

using std::deque;
using std::string;
using std::vector;

namespace Hardware { namespace Protocols
{
	void MaerklinCANCommon::Init()
	{
		run = true;
		receiverThread = std::thread(&MaerklinCANCommon::ReceiverInternal, this);
		pingThread = std::thread(&MaerklinCANCommon::PingSender, this);
	}

	MaerklinCANCommon::~MaerklinCANCommon()
	{
		Stop();
		while (canFiles.size())
		{
			vector<struct CanFile>::iterator firstCanFile = canFiles.begin();
			DeleteCanFile(&(*firstCanFile));
		}
	}

	void MaerklinCANCommon::Stop()
	{
		run = false;
		if (receiverThread.joinable())
		{
			receiverThread.join();
		}
		if (pingThread.joinable())
		{
			pingThread.join();
		}
		pendingMfxBindings.clear();
		pendingMfxVerifications.clear();
	}

	void MaerklinCANCommon::Wait(const unsigned int duration) const
	{
		unsigned int wait = duration;
		while (run && (!hasCs2Main || isCs2MainEmulation) && wait)
		{
			Utils::Utils::SleepForSeconds(1);
			--wait;
		}
	}

	void MaerklinCANCommon::ReceiverInternal()
	{
		Utils::Utils::SetThreadName("Maerklin CAN Receiver");
		logger->Info(Languages::TextReceiverThreadStarted);
		Receiver();
		logger->Info(Languages::TextTerminatingReceiverThread);
	}

	void MaerklinCANCommon::PingSender()
	{
		Utils::Utils::SetThreadName("Maerklin CAN Ping");
		logger->Info(Languages::TextPingSenderStarted);

		Wait(1);

		bool discoverySent = false;
		while (run && (!hasCs2Main || isCs2MainEmulation))
		{
			SendBootloaderCan();
			Utils::Utils::SleepForMilliseconds(100);
			SendPing();
			if (isCs2MainEmulation)
			{
				// The MS2 starts discovery with a one-byte, zero-range request.
				// Initiate it once after the booster is on so the Gleisbox can report new mfx UIDs.
				if (!discoverySent && manager->Booster() == BoosterStateGo)
				{
					Wait(1);
					VerifyAllMfxLocos();
					Wait(1);
//					SendDiscovery();
					discoverySent = true;
				}
			}
			Wait(20);
		}

		if (run && hasCs2Main && !isCs2MainEmulation)
		{
			RequestLoks();
		}
		logger->Info(Languages::TextTerminatingPingSender);
	}

	void MaerklinCANCommon::CreateCommandHeader(unsigned char* const buffer,
		const CanCommand command,
		const CanResponse response,
		const CanLength length)
	{
		const CanPrio prio = 0;
		buffer[0] = (prio << 1) | (command >> 7);
		buffer[1] = (command << 1) | (response & 0x01);
		Utils::Integer::ShortToDataBigEndian(hash, buffer + 2);
		buffer[4] = length;
		std::memset(buffer + 5, 0, 8);
	}

	void MaerklinCANCommon::ParseAddressProtocol(const Address input,
		Address& address,
		Protocol& protocol,
		LocoType& type)
	{
		address = input;
		Address maskedAddress = address & 0xFC00;
		type = LocoTypeLoco;

		if ((maskedAddress == 0x0000)     // MM Loco
			|| (maskedAddress == 0x1000)  // MM Loco (unused by CS2)
			|| (maskedAddress == 0x2000)  // MM Accessory (unused by CS2)
			|| (maskedAddress == 0x3000)) // MM Accessory
		{
			protocol = ProtocolMM;
			address &= 0x03FF;
			return;
		}

		// 0x0800 is SX1 Loco (unused by CS2)
		// 0x2800 is SX1 Accessory (unused by CS2)

		if (maskedAddress == 0x2C00) // Multiple Unit
		{
			protocol = ProtocolMultipleUnit;
			type = LocoTypeMultipleUnit;
			address &= 0x03FF;
			return;
		}

		if ((maskedAddress == 0x3800)     // DCC Accessory
			|| (maskedAddress == 0x3C00)) // DCC Accessory
		{
			protocol = ProtocolDCC;
			address &= 0x03FF;
			return;
		}

		maskedAddress = address & 0xC000;
		address &= 0x3FFF;
		if (maskedAddress == 0x4000) // MFX Loco
		{
			protocol = ProtocolMFX;
			return;
		}
		if (maskedAddress == 0xC000) // DCC Loco
		{
			protocol = ProtocolDCC;
			return;
		}

		protocol = ProtocolNone;
		address = 0;
	}

	const char* MaerklinCANCommon::DeviceTypeAsString(const uint16_t deviceType)
	{
		switch (deviceType)
		{
			case CanDeviceGfp:
				return "Gleisformat Prozessor";

			case CanDeviceGleisbox:
			case CanDeviceGleisbox_2:
				return "Gleisbox";

			case CanDeviceConnect6021:
				return "Connect 6021";

			case CanDeviceMs2:
			case CanDeviceMs2_2:
			case CanDeviceMs2_3:
			case CanDeviceMs2_4:
				return "MS2";

			case CanDeviceWireless:
				return "Wireless";

			case CanDeviceCs2Main:
				return "CS2 Main";

			case CanDeviceCs2Secondary:
			case CanDeviceCs2Secondary_2:
				return "CS2 Secondary";

			case CanDeviceLinkS88:
				return "Link S88";

			default:
				return "unknown";
		}
	}

	MaerklinCANCommon::CanFileCrc MaerklinCANCommon::CalcCrc(const unsigned char *data, const size_t length)
	{
		// Maerklin uses CRC-16-CCITT with seed 0xffff and with polynom 0x1021 -> x^16 + x^12 +x^5 + 1
		static const CanFileCrc CrcTable[256] = {
			0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
			0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
			0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
			0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
			0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
			0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
			0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
			0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
			0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
			0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
			0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
			0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
			0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
			0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
			0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
			0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
			0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
			0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
			0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
			0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
			0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
			0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
			0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
			0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
			0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
			0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
			0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
			0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
			0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
			0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
			0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
			0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
		};

		CanFileCrc crc = 0xffff; // seed

		for (size_t count = 0; count < length; ++count)
		{
			const CanFileCrc temp = (*data++ ^ (crc >> 8)) & 0xff;
			crc = CrcTable[temp] ^ (crc << 8);
		}
		return crc;
	}

	MaerklinCANCommon::CanHash MaerklinCANCommon::CalcHash(const CanUid uid)
	{
		const CanHash calc = (uid >> 16) ^ (uid & 0xFFFF);
		CanHash hash = ((calc << 3) | 0x0300) & 0xFF00;
		hash |= (calc & 0x007F);
		return hash;
	}

	void MaerklinCANCommon::GenerateUidHash()
	{
		uid = rand();
		string uidString = Utils::Integer::IntegerToHex(uid);
		// FIXME: params->SetArg5(uidString);
		hash = CalcHash(uid);
		logger->Info(Languages::TextMaerklinCanStarting, uidString, Utils::Integer::IntegerToHex(hash), Languages::GetOnOff(isCs2MainEmulation));
	}

	void MaerklinCANCommon::CreateLocalIDLoco(unsigned char* buffer,
		const Protocol protocol,
		const Address address) const
	{
		uint32_t localID = address;
		if (protocol == ProtocolDCC)
		{
			localID |= 0xC000;
		}
		else if (protocol == ProtocolMFX)
		{
			localID |= 0x4000;
		}
		else if (protocol == ProtocolMultipleUnit)
		{
			localID |= 0x2000;
		}
		// else expect PROTOCOL_MM2: do nothing
		Utils::Integer::IntToDataBigEndian(localID, buffer + 5);
	}

	void MaerklinCANCommon::CreateLocalIDAccessory(unsigned char* buffer,
		const Protocol protocol,
		const Address address) const
	{
		uint32_t localID = address - 1; // GUI-address is 1-based, protocol-address is 0-based
		if (protocol == ProtocolDCC)
		{
			localID |= 0x3800;
		}
		else // ProtocolMM
		{
			localID |= 0x3000;
		}
		Utils::Integer::IntToDataBigEndian(localID, buffer + 5);
	}

	void MaerklinCANCommon::Booster(const BoosterState status)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(status ? Languages::TextTurningBoosterOn : Languages::TextTurningBoosterOff);
		CreateCommandHeader(buffer, CanCommandSystem, CanResponseCommand, 5);
		buffer[9] = status;
		SendInternal(buffer);
	}

	void MaerklinCANCommon::LocoSpeed(const Protocol protocol, const Address address, const Speed speed)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingSpeedWithProtocol, Utils::Utils::ProtocolToString(protocol), address, speed);
		CreateCommandHeader(buffer, CanCommandLocoSpeed, CanResponseCommand, 6);
		CreateLocalIDLoco(buffer, protocol, address);
		Utils::Integer::ShortToDataBigEndian(speed, buffer + 9);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::LocoOrientation(const Protocol protocol, const Address address,
		const Orientation orientation)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingDirectionOfTravelWithProtocol, Utils::Utils::ProtocolToString(protocol), address, Languages::GetLeftRight(orientation));
		CreateCommandHeader(buffer, CanCommandLocoDirection, CanResponseCommand, 5);
		CreateLocalIDLoco(buffer, protocol, address);
		buffer[9] = (orientation ? 1 : 2);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::LocoFunction(const Protocol protocol,
		const Address address,
		const DataModel::LocoFunctionNr function,
		const DataModel::LocoFunctionState on)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingFunctionWithProtocol, static_cast<int>(function), Utils::Utils::ProtocolToString(protocol), address, Languages::GetOnOff(on));
		CreateCommandHeader(buffer, CanCommandLocoFunction, CanResponseCommand, 6);
		CreateLocalIDLoco(buffer, protocol, address);
		buffer[9] = function;
		buffer[10] = (on == DataModel::LocoFunctionStateOn);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::Accessory(const Protocol protocol,
		const Address address,
		const DataModel::AccessoryState state,
		const bool on)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingAccessoryWithProtocol, Utils::Utils::ProtocolToString(protocol), address, Languages::GetGreenRed(state), Languages::GetOnOff(on));
		CreateCommandHeader(buffer, CanCommandAccessory, CanResponseCommand, 6);
		CreateLocalIDAccessory(buffer, protocol, address);
		buffer[9] = state & 0x03;
		buffer[10] = static_cast<unsigned char>(on);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::ProgramReadMfx(const Address address)
	{
		DecoderHandlerMfx::ReadMfxDecoder(address);
	}

	void MaerklinCANCommon::ProgramRead(const ProgramMode mode,
		const Address address,
		const CvNumber cv)
	{
		Address addressInternal = address;
		Protocol protocol = ProtocolNone;
		switch (mode)
		{
			case ProgramModeDccDirect:
				logger->Info(Languages::TextProgramDccDirectRead, cv);
				protocol = ProtocolDCC;
				break;

			case ProgramModeMfx:
				logger->Info(Languages::TextProgramMfxRead, address, cv & 0x3FF, cv >> 10);
				protocol = ProtocolMFX;
				break;

			default:
				return;
		}
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandReadConfig, CanResponseCommand, 7);
		CreateLocalIDLoco(buffer, protocol, addressInternal);
		Utils::Integer::ShortToDataBigEndian(cv, buffer + 9);
		buffer[11] = 1;
		SendInternal(buffer);
	}

	void MaerklinCANCommon::ProgramWrite(const ProgramMode mode,
		const Address address,
		const CvNumber cv,
		const CvValue value)
	{
		Address addressInternal = address;
		Protocol protocol = ProtocolNone;
		unsigned char controlFlags = 0;
		switch (mode)
		{
			case ProgramModeMm:
				logger->Info(Languages::TextProgramMm, cv, value);
				protocol = ProtocolMM;
				addressInternal = 80;
				break;

			case ProgramModeMmPom:
				logger->Info(Languages::TextProgramMmPom, address, cv, value);
				protocol = ProtocolMM;
				controlFlags = 1 << 7;
				break;

			case ProgramModeDccDirect:
				logger->Info(Languages::TextProgramDccDirectWrite, cv, value);
				protocol = ProtocolDCC;
				addressInternal = 0;
				break;

			case ProgramModeDccPomLoco:
				logger->Info(Languages::TextProgramDccPomLocoWrite, address, cv, value);
				protocol = ProtocolDCC;
				controlFlags = 1 << 7;
				break;

			case ProgramModeMfx:
				logger->Info(Languages::TextProgramMfxWrite, address, cv & 0x3FF, cv >> 10, value);
				protocol = ProtocolMFX;
				controlFlags = 1 << 7;
				break;

			default:
				return;
		}
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandWriteConfig, CanResponseCommand, 8);
		CreateLocalIDLoco(buffer, protocol, addressInternal);
		Utils::Integer::ShortToDataBigEndian(cv, buffer + 9);
		buffer[11] = value;
		buffer[12] = controlFlags;
		SendInternal(buffer);
	}

	void MaerklinCANCommon::SendPing()
	{
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandPing, CanResponseCommand, 0);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::IncrementMfxCounter()
	{
		++mfxCounter;
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandSystem, CanResponseCommand, 1);
		buffer[9] = CanSubCommandMfxCounter;
		Utils::Integer::ShortToDataBigEndian(mfxCounter, buffer + 10);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::SendBootloaderCan()
	{
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandBootloaderCan, CanResponseCommand, 0);
		SendInternal(buffer);
	}

	void MaerklinCANCommon::SendDiscovery()
	{
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandDiscovery, CanResponseCommand, 1);
		buffer[5] = 0; // mfx programming track discovery
		SendInternal(buffer);
	}

	void MaerklinCANCommon::VerifyAllMfxLocos()
	{
		// Verify only locomotives assigned to this controller.
		const vector<std::pair<MfxUid,Address>> mfxLocos = manager->LocoMfxUidsSids(controlID);
		for (const std::pair<MfxUid,Address>& mfxLoco : mfxLocos)
		{
			{
				const std::lock_guard<std::mutex> lock(pendingMfxVerificationsMutex);
				pendingMfxVerifications[mfxLoco.first] = mfxLoco.second;
			}
			SendMfxVerify(mfxLoco.first, mfxLoco.second);
			Wait(5);
		}
	}

	void MaerklinCANCommon::RequestLoks()
	{
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandRequestConfigData, CanResponseCommand, 8);
		buffer[5] = 'l';
		buffer[6] = 'o';
		buffer[7] = 'k';
		buffer[8] = 's';
		buffer[9] = 0;
		buffer[10] = 0;
		buffer[11] = 0;
		buffer[12] = 0;
		SendInternal(buffer);
	}

	void MaerklinCANCommon::Parse(const unsigned char* buffer)
	{
		CanResponse response = ParseResponse(buffer);
		CanCommand command = ParseCommand(buffer);
		CanLength length = ParseLength(buffer);
		logger->HexIn(buffer, 5 + length);
		const CanHash receivedHash = ParseHash(buffer);
		if (receivedHash == hash)
		{
			if (command == CanCommandPing && response)
			{
				uint16_t deviceType = Utils::Integer::DataBigEndianToShort(buffer + 11);
				if (deviceType == CanDeviceCs2Main)
				{
					hasCs2Main = true;
					logger->Info(Languages::TextCs2MainFound);
				}
			}
			else if (command != CanCommandConfigData)
			{
				GenerateUidHash();
			}
		}

		if (response)
		{
			switch (command)
			{
				case CanCommandDiscovery:
					ParseResponseDiscovery(buffer);
					return;

				case CanCommandMfxBind:
					ParseResponseMfxBind(buffer);
					return;

				case CanCommandMfxVerify:
					ParseResponseMfxVerify(buffer);
					return;

				case CanCommandS88Event:
					ParseResponseS88Event(buffer);
					return;

				case CanCommandReadConfig:
					ParseResponseReadConfig(buffer);
					return;

				case CanCommandPing:
					ParseResponsePing(buffer);
					return;

				case CanCommandBootloaderCan:
					ParseResponseBootloaderCan(buffer);
					return;

				case CanCommandLocoSpeed:
					ParseResponseLocoSpeed(buffer);
					return;

				case CanCommandLocoDirection:
					ParseResponseLocoDirection(buffer);
					return;

				case CanCommandLocoFunction:
					ParseResponseLocoFunction(buffer);
					return;

				case CanCommandAccessory:
					ParseResponseAccessory(buffer);
					return;

				default:
					return;
			}
		}

		switch (command)
		{
			case CanCommandSystem:
				ParseCommandSystem(buffer);
				return;

			case CanCommandDiscovery:
				ParseResponseDiscovery(buffer);  // FIXME: This is a response, not a command. The function name is misleading.
				return;

			case CanCommandMfxBind:
				ParseResponseMfxBind(buffer); // FIXME: This is a response, not a command. The function name is misleading.
				return;

			case CanCommandMfxVerify:
				ParseResponseMfxVerify(buffer); // FIXME: This is a response, not a command. The function name is misleading.
				return;

			case CanCommandRequestConfigData:
				ParseCommandRequestConfigData(buffer);
				return;

			case CanCommandConfigData:
				ParseCommandConfigData(buffer);
				return;

			case CanCommandPing:
				ParseCommandPing(buffer);
				return;

			case CanCommandBootloaderCan:
				ParseCommandBootloaderCan(buffer);
				return;

			default:
				return;
		}
	}

	static string MaerklinCANResponseString(const unsigned char response)
	{
		return response ? "response" : "command";
	}

	static string MaerklinCANHeaderDetails(const unsigned char* const buffer)
	{
		return "type=" + MaerklinCANResponseString(buffer[1] & 0x01)
			+ ", hash=0x" + Utils::Integer::IntegerToHex(Utils::Integer::DataBigEndianToShort(buffer + 2), 4)
			+ ", dlc=" + std::to_string(buffer[4]);
	}

	static string MaerklinCANDiscoveryProtocol(const uint8_t protocol)
	{
		if (protocol <= 32)
		{
			return "mfx programming track range=" + std::to_string(protocol);
		}
		if (protocol >= 64 && protocol <= 96)
		{
			return "mfx main track range=" + std::to_string(protocol);
		}

		switch (protocol)
		{
			case 33:
				return "MM2 programming track discovery 20 kHz";

			case 34:
				return "MM2 programming track discovery 40 kHz";

			case 35:
				return "DCC programming track short address read";

			case 36:
				return "DCC programming track long address read";

			case 37:
				return "DCC programming track discovery";

			case 38:
				return "SX1 programming track address read";

			case 39:
				return "SX1 programming track discovery";

			case 98:
				return "MM2 main track invalid";

			case 99:
				return "DCC main track invalid short address read";

			case 100:
				return "DCC main track invalid long address read";

			case 101:
				return "SX1 main track invalid address read";

			case 102:
				return "SX1 main track invalid discovery";
		}
		return "unknown protocol/range=" + std::to_string(protocol);
	}

	void MaerklinCANCommon::ParseResponseDiscovery(const unsigned char* const buffer)
	{
		const CanLength length = ParseLength(buffer);
		string details = MaerklinCANHeaderDetails(buffer);

		switch (length)
		{
			case 0:
				details += ", form=1, discovery=all protocols";
				break;

			case 1:
				details += ", form=2, " + MaerklinCANDiscoveryProtocol(buffer[5]);
				break;

			case 5:
			case 6:
			{
				const uint8_t protocol = buffer[9];
				const uint32_t id = Utils::Integer::DataBigEndianToInt(buffer + 5);
				details += ", form=3, " + MaerklinCANDiscoveryProtocol(protocol);
				if (MaerklinCANDiscoveryIsMfx(protocol))
				{
					details += ", mfxUid=0x" + Utils::Integer::IntegerToHex(id, 8);
				}
				else
				{
					Address address;
					Protocol parsedProtocol;
					LocoType type;
					ParseAddressProtocol(id, address, parsedProtocol, type);
					details += ", locId=0x" + Utils::Integer::IntegerToHex(id, 8)
						+ ", protocol=" + Utils::Utils::ProtocolToString(parsedProtocol)
						+ ", address=" + std::to_string(address);
				}
				if (length == 6)
				{
					details += ", ask=0x" + Utils::Integer::IntegerToHex(buffer[10], 2)
						+ " (" + std::to_string(buffer[10]) + ")";
				}
				break;
			}

			default:
				details += ", unsupported discovery length";
				break;
		}
		logger->Debug(Languages::TextMaerklinCANFrameReceived, "Discovery", details);
		// DLC 6 contains an intermediate ASK; only DLC 5 reports a complete UID.
		if (!isCs2MainEmulation || (length != 5))
		{
			return;
		}

		const MfxUid mfxUid = ParseUid(buffer);
		logger->Info(Languages::TextMfxLocoDetected, Utils::Integer::IntegerToHex(mfxUid, 8));

		const Address mfxSid = RegisterMfxDiscovery(mfxUid);
		SendMfxBind(mfxUid, mfxSid);
	}

	Address MaerklinCANCommon::RegisterMfxDiscovery(const MfxUid mfxUid)
	{
		const DataModel::LocoConfig loco = manager->GetLocoByMfxUid(mfxUid);
		const Address sid = (loco.GetType() != LocoTypeLoco ? manager->GetFreeMfxSid() : loco.GetAddress());
		pendingMfxBindings[mfxUid] = sid;
		return sid;
	}

	bool MaerklinCANCommon::IsPendingMfxBinding(const MfxUid mfxUid, const Address sid) const
	{
		const std::map<MfxUid,Address>::const_iterator pending = pendingMfxBindings.find(mfxUid);
		return ((pending != pendingMfxBindings.end()) && (pending->second == sid));
	}

	void MaerklinCANCommon::SendMfxBind(const MfxUid mfxUid, const Address mfxSid)
	{
		unsigned char sendBuffer[CANCommandBufferLength];
		CreateCommandHeader(sendBuffer, CanCommandMfxBind, CanResponseCommand, 6);
		Utils::Integer::IntToDataBigEndian(mfxUid, sendBuffer + 5);
		Utils::Integer::ShortToDataBigEndian(mfxSid, sendBuffer + 9);
		SendInternal(sendBuffer);
		logger->Debug(Languages::TextMfxBindSent, Utils::Integer::IntegerToHex(mfxUid, 8), mfxSid);
	}

	void MaerklinCANCommon::ParseResponseMfxBind(const unsigned char* const buffer)
	{
		string details = MaerklinCANHeaderDetails(buffer);
		if (ParseLength(buffer) == 6)
		{
			const uint32_t uid = Utils::Integer::DataBigEndianToInt(buffer + 5);
			const uint16_t sid = Utils::Integer::DataBigEndianToShort(buffer + 9);
			details += ", mfxUid=0x" + Utils::Integer::IntegerToHex(uid, 8)
				+ ", mfxSid=0x" + Utils::Integer::IntegerToHex(sid, 4)
				+ " (" + std::to_string(sid) + ")";
		}
		else
		{
			details += ", unsupported bind length";
		}
		logger->Debug(Languages::TextMaerklinCANFrameReceived, "MFX Bind", details);

		if (!isCs2MainEmulation || ParseLength(buffer) != 6)
		{
			return;
		}
		const MfxUid mfxUid = ParseUid(buffer);
		const Address mfxSid = Utils::Integer::DataBigEndianToShort(buffer + 9);
		if (!IsPendingMfxBinding(mfxUid, mfxSid))
		{
			return;
		}

		SendMfxVerify(mfxUid, mfxSid);
	}

	void MaerklinCANCommon::SendMfxVerify(const MfxUid mfxUid, const Address mfxSid)
	{
		unsigned char sendBuffer[CANCommandBufferLength];
		CreateCommandHeader(sendBuffer, CanCommandMfxVerify, CanResponseCommand, 6);
		Utils::Integer::IntToDataBigEndian(mfxUid, sendBuffer + 5);
		Utils::Integer::ShortToDataBigEndian(mfxSid, sendBuffer + 9);
		SendInternal(sendBuffer);
		logger->Debug(Languages::TextMfxVerifySent, Utils::Integer::IntegerToHex(mfxUid, 8), mfxSid);
	}

	void MaerklinCANCommon::ParseResponseMfxVerify(const unsigned char* const buffer)
	{
		const CanLength length = ParseLength(buffer);
		string details = MaerklinCANHeaderDetails(buffer);
		if (length == 6 || length == 7)
		{
			const uint32_t uid = Utils::Integer::DataBigEndianToInt(buffer + 5);
			const uint16_t sid = Utils::Integer::DataBigEndianToShort(buffer + 9);
			details += ", mfxUid=0x" + Utils::Integer::IntegerToHex(uid, 8)
				+ ", mfxSid=0x" + Utils::Integer::IntegerToHex(sid, 4)
				+ " (" + std::to_string(sid) + ")";
			if (ParseResponse(buffer) == CanResponseResponse)
			{
				details += sid == 0 ? ", result=negative" : ", result=positive";
			}
			if (length == 7)
			{
				details += ", ask=0x" + Utils::Integer::IntegerToHex(buffer[11], 2);
			}
		}
		else
		{
			details += ", unsupported verify length";
		}
		logger->Debug(Languages::TextMaerklinCANFrameReceived, "MFX Verify", details);
		if (!isCs2MainEmulation
			|| (ParseResponse(buffer) != CanResponseResponse)
			|| ((length != 6) && (length != 7)))
		{
			return;
		}

		const MfxUid mfxUid = ParseUid(buffer);
		const Address mfxSid = Utils::Integer::DataBigEndianToShort(buffer + 9);
		Address bindSid = AddressNone;
		{
			const std::lock_guard<std::mutex> lock(pendingMfxVerificationsMutex);
			const std::map<MfxUid,Address>::iterator pending = pendingMfxVerifications.find(mfxUid);
			if ((pending != pendingMfxVerifications.end())
				&& ((mfxSid == AddressNone) || (pending->second == mfxSid)))
			{
				if (mfxSid == AddressNone)
				{
					bindSid = pending->second;
				}
				// Consume the initial verification before binding: never retry a failed post-bind verification.
				pendingMfxVerifications.erase(pending);
			}
		}
		if (mfxSid == AddressNone)
		{
			if (bindSid != AddressNone)
			{
				pendingMfxBindings[mfxUid] = bindSid;
				SendMfxBind(mfxUid, bindSid);
			}
			else
			{
				pendingMfxBindings.erase(mfxUid);
			}
			return;
		}
		if (!IsPendingMfxBinding(mfxUid, mfxSid))
		{
			return;
		}

		pendingMfxBindings.erase(mfxUid);

		// Request reading the MFX data for the newly bound locomotive
		ReadMfxDecoder(mfxSid);
	}

	void MaerklinCANCommon::ParseCommandSystem(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 5)
		{
			return;
		}
		CanSubCommand subcmd = ParseSubCommand(buffer);
		switch (subcmd)
		{
			case CanSubCommandStop:
				// system stop
				manager->Booster(ControlTypeHardware, BoosterStateStop);
				return;

			case CanSubCommandGo:
				// system go
				manager->Booster(ControlTypeHardware, BoosterStateGo);
				return;

			case CanSubCommandMfxCounter:
				// we ignore mfx counter
				return;
		}
	}

	void MaerklinCANCommon::ParseResponseLocoSpeed(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 6)
		{
			return;
		}
		Address address;
		Protocol protocol;
		LocoType type;
		ParseAddressProtocol(buffer, address, protocol, type);
		Speed speed = Utils::Integer::DataBigEndianToShort(buffer + 9);
		logger->Info(Languages::TextReceivedSpeedCommand, Utils::Utils::ProtocolToString(protocol), address, speed);
		manager->LocoSpeed(ControlTypeHardware, controlID, protocol, address, speed);
	}

	void MaerklinCANCommon::ParseResponseLocoDirection(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 5)
		{
			return;
		}
		Address address;
		Protocol protocol;
		LocoType type;
		ParseAddressProtocol(buffer, address, protocol, type);
		Orientation orientation = (buffer[9] == 1 ? OrientationRight : OrientationLeft);
		logger->Info(Languages::TextReceivedDirectionCommand, Utils::Utils::ProtocolToString(protocol), address, orientation);
		// changing direction implies speed = 0
		manager->LocoSpeed(ControlTypeHardware, controlID, protocol, address, MinSpeed);
		manager->LocoOrientation(ControlTypeHardware, controlID, protocol, address, orientation);
	}

	void MaerklinCANCommon::ParseResponseLocoFunction(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 6)
		{
			return;
		}
		Address address;
		Protocol protocol;
		LocoType type;
		ParseAddressProtocol(buffer, address, protocol, type);
		DataModel::LocoFunctionNr function = buffer[9];
		DataModel::LocoFunctionState on = (buffer[10] != 0 ? DataModel::LocoFunctionStateOn : DataModel::LocoFunctionStateOff);
		logger->Info(Languages::TextReceivedFunctionCommand, Utils::Utils::ProtocolToString(protocol), address, function, on);
		manager->LocoFunctionState(ControlTypeHardware, controlID, protocol, address, function, on);
	}

	void MaerklinCANCommon::ParseResponseAccessory(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 6 || buffer[10] != 1)
		{
			return;
		}
		Address address;
		Protocol protocol;
		LocoType type;
		ParseAddressProtocol(buffer, address, protocol, type);
		DataModel::AccessoryState state = (buffer[9] ? DataModel::AccessoryStateOn : DataModel::AccessoryStateOff);
		// GUI-address is 1-based, protocol-address is 0-based
		++address;
		logger->Info(Languages::TextReceivedAccessoryCommand, Utils::Utils::ProtocolToString(protocol), address, state);
		manager->AccessoryBaseState(ControlTypeHardware, controlID, protocol, address, state);
	}

	void MaerklinCANCommon::ParseCommandPing(const unsigned char* const buffer)
	{
		if ((ParseLength(buffer) == 8) && (Utils::Integer::DataBigEndianToInt(buffer + 5) != uid))
		{
			return;
		}
		unsigned char sendBuffer[CANCommandBufferLength];
		CreateCommandHeader(sendBuffer, CanCommandPing, CanResponseResponse, 8);
		Utils::Integer::IntToDataBigEndian(uid, sendBuffer + 5);

		// CS2 version to emulate
		sendBuffer[9] = EmulateCS2VersionMajor;
		sendBuffer[10] = EmulateCS2VersionMinor;

		// CS2 device type: Main = 0xffff, Secondary = 0xfff0
		sendBuffer[11] = 0xff;
		sendBuffer[12] = isCs2MainEmulation ? 0xff : 0xf0;
		SendInternal(sendBuffer);
		logger->Debug(Languages::TextSendingPingResponse, Utils::Integer::IntegerToHex(uid, 8),  Languages::GetText(isCs2MainEmulation ? Languages::TextCS2MainDevice : Languages::TextCS2SecondaryDevice));
	}

	void MaerklinCANCommon::ParseCommandBootloaderCan(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 0)
		{
			return;
		}

	}

	void MaerklinCANCommon::ParseResponseBootloaderCan(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 8)
		{
			return;
		}

		const uint16_t deviceType = Utils::Integer::DataBigEndianToShort(buffer + 11);
		const string hash = Utils::Integer::IntegerToHex(Utils::Integer::DataBigEndianToShort(buffer + 2));
		const unsigned char deviceId = buffer[8];
		const unsigned char majorVersion = buffer[9];
		const unsigned char minorVersion = buffer[10];
		logger->Debug(Languages::TextBootloaderCanUpdateRequest, DeviceTypeAsString(deviceType), hash, deviceId, majorVersion, minorVersion);

		// answer with the same UID is required to satisfy the bootloader
		unsigned char sendBuffer[CANCommandBufferLength];
		CreateCommandHeader(sendBuffer, CanCommandBootloaderCan, CanResponseCommand, 5);
		std::memcpy(sendBuffer + 5, buffer + 5, 4); // echo back the UID
		sendBuffer[9] = 0x11; // unknown, but seems to satisfy the bootloader
		SendInternal(sendBuffer);
	}

	string MaerklinCANCommon::GetMs2LocoNames(const size_t offset, const size_t count) const
	{
		const std::map<string,LocoID> locoIds = manager->LocoIdsByName();
		string out("[lokomotive]");
		size_t index = 0;
		size_t sent = 0;
		for (const auto& loco : locoIds)
		{
			if (index < offset)
			{
				++index;
				continue;
			}
			if (sent >= count)
			{
				break;
			}
			out += "\nlok\n .nr=" + std::to_string(index);
			out += "\n .name=" + loco.first;
			++index;
			++sent;
		}
		out += "\nnumloks\n .wert=" + std::to_string(locoIds.size());
		return out + "\n";
	}

	string MaerklinCANCommon::GetMs2LocoInfo(const string& name) const
	{
		const DataModel::LocoConfig loco = manager->GetLocoByName(name);
		if (!loco.IsValid())
		{
			return string();
		}

		uint32_t uid = loco.GetAddress();
		string protocol;
		switch (loco.GetProtocol())
		{
			case ProtocolMM:
				protocol = "mm2";
				break;

			case ProtocolDCC:
				uid += 0xC000;
				protocol = "dcc";
				break;

			case ProtocolMFX:
				uid += 0x4000;
				protocol = "mfx";
				break;

			default:
				protocol = "unknown";
				break;
		}

		string out("[lokomotive]\nlok");
		out += "\n .uid=0x" + Utils::Integer::IntegerToHex(uid);
		out += "\n .name=" + loco.GetName();
		out += "\n .adresse=0x" + Utils::Integer::IntegerToHex(loco.GetAddress());
		out += "\n .typ=" + protocol;
		if (loco.GetProtocol() == ProtocolMFX)
		{
			out += "\n .mfxsid=0x" + Utils::Integer::IntegerToHex(loco.GetAddress());
			out += "\n .mfxuid=0x" + Utils::Integer::IntegerToHex(loco.GetMfxUid(), 8);
		}
		out += "\n .volume=100";
		out += "\n .vmax=" + std::to_string(loco.GetMaxSpeed() >> 2); // vmax is 0..255, speed is 0..1023, so divide by 4
		out += "\n .vmin=1";

		DataModel::LocoFunctionEntry functions[DataModel::NumberOfLocoFunctions];
		loco.GetFunctionStates(functions);
		for (DataModel::LocoFunctionNr nr = 0; nr < 16; ++nr)
		{
			out += "\n .fkt\n ..nr=";
			out += std::to_string(nr);
			out += "\n ..typ=";
			out += std::to_string(MapLocofunctionRailControlToCs2(nr, functions[nr].icon));
		}
		for (DataModel::LocoFunctionNr nr = 16; nr <= DataModel::NumberOfLocoFunctions; ++nr)
		{
			out += "\n .fkt2\n ..nr2=";
			out += std::to_string(nr);
			out += "\n ..typ2=";
			out += std::to_string(MapLocofunctionRailControlToCs2(nr, functions[nr].icon));
		}
		return out + "\n";
	}

	void MaerklinCANCommon::ParseCommandRequestConfigData(const unsigned char* const buffer)
	{
		if (!isCs2MainEmulation || ParseLength(buffer) != 8)
		{
			return;
		}
		const CanHash requestHash = ParseHash(buffer);
		// The first 8-byte request names the file. Loknamen, lokinfo and version
		// files need a follow-up frame. Keep each request by sender hash until
		// its follow-up arrives, then send the configuration.
		const auto pendingRequest = configRequests.find(requestHash);
		if (pendingRequest != configRequests.end())
		{
			if (pendingRequest->second.type == ConfigRequestVersion)
			{
				// Version requests end with eight ASCII 'f' bytes, not 0xff bytes.
				if (std::memcmp(buffer + 5, "ffffffff", 8) == 0)
				{
					const string& request = pendingRequest->second.data;
					SendUncompressedFile(request.substr(8),
						reinterpret_cast<const unsigned char*>(request.data()), requestHash);
				}
				configRequests.erase(pendingRequest);
				return;
			}
			if (pendingRequest->second.type == ConfigRequestLocoNames)
			{
				// The next frame supplies the offset and number of names to send.
				const string parameters(reinterpret_cast<const char*>(buffer + 5), 8);
				std::istringstream parameterStream(parameters);
				size_t offset;
				size_t count;
				if (parameterStream >> offset >> count)
				{
					const string data = GetMs2LocoNames(offset, count);
					SendUncompressedFile(data, buffer + 5, requestHash);
				}
				configRequests.erase(pendingRequest);
				return;
			}

			// A zero-filled frame ends the lokinfo name, which may span several frames.
			bool endOfRequest = true;
			for (size_t i = 0; i < 8; ++i)
			{
				if (buffer[5 + i] != 0)
				{
					endOfRequest = false;
					break;
				}
			}
			if (endOfRequest)
			{
				const string data = GetMs2LocoInfo(pendingRequest->second.data);
				SendUncompressedFile(data, buffer + 5, requestHash);
				configRequests.erase(pendingRequest);
				return;
			}
			for (size_t i = 0;
				i < 8 && buffer[5 + i] != 0 && pendingRequest->second.data.size() < 255;
				++i)
			{
				pendingRequest->second.data += static_cast<char>(buffer[5 + i]);
			}
			return;
		}

		if (std::memcmp(buffer + 5, "loknamen", 8) == 0)
		{
			configRequests.emplace(requestHash, ConfigRequest{ConfigRequestLocoNames, string()});
			return;
		}
		if (std::memcmp(buffer + 5, "lokinfo\0", 8) == 0)
		{
			configRequests.emplace(requestHash, ConfigRequest{ConfigRequestLocoInfo, string()});
			return;
		}
		if (std::memcmp(buffer + 5, "ldbver\0\0", 8) == 0)
		{
			const std::time_t now = std::time(nullptr);
			std::tm localTime = {};
			localtime_r(&now, &localTime);
			string data(" .version=16"); // FIXME: this is the version of the loco database, but we don't have a way to get it from the DataModel, so just return 16 for now
			data += "\n .monat=" + std::to_string(localTime.tm_mon + 1);
			data += "\n .jahr=" + std::to_string(localTime.tm_year + 1900);
			data += "\n .anzahl=" + std::to_string(manager->NumberOfLocos());
			configRequests.emplace(requestHash, ConfigRequest{ConfigRequestVersion,
				string(reinterpret_cast<const char*>(buffer + 5), 8) + data + "\n"});
			return;
		}

		struct VersionFile
		{
			unsigned char name[8];
			const char* data;
		};
		static const VersionFile versionFiles[] =
		{
			{{'l', 'a', 'n', 'g', 'v', 'e', 'r', 0x00}, " .vhigh=4\n .vlow=1\n .bytes=68223\n"},
			{{'m', 'f', 'x', 'v', 'e', 'r', 0x00, 0x00}, " .vhigh=0\n .vlow=3\n .bytes=1902\n"},
			{{'m', 'f', 'x', 'b', 'v', 'e', 'r', 0x00}, " .vhigh=0\n .vlow=3\n .bytes=772\n"},
			{{'m', 's', '2', 'v', 'e', 'r', 0x00, 0x00}, " .vhigh=4\n .vlow=1\n .bytes=92932\n"},
			{{'m', 's', '2', 'x', 'v', 'e', 'r', 0x00}, " .vhigh=4\n .vlow=1\n .bytes=155003\n"},
			{{'m', 's', '2', 'y', 'v', 'e', 'r', 0x00}, " .vhigh=4\n .vlow=1\n .bytes=122880\n"},
			{{'g', 'b', '2', 'v', 'e', 'r', 0x00, 0x00}, " .vhigh=1\n .vlow=62\n .bytes=26648\n"}
		};
		for (const VersionFile& versionFile : versionFiles)
		{
			if (std::memcmp(buffer + 5, versionFile.name, 8) == 0)
			{
				configRequests.emplace(requestHash, ConfigRequest{ConfigRequestVersion,
					string(reinterpret_cast<const char*>(buffer + 5), 8) + versionFile.data});
				return;
			}
		}

		if (std::memcmp(buffer + 5, "loks\0\0\0\0", 8) == 0)
		{
			const string dataPlain = manager->GetCs2Lokomotive();
			SendCompressedFile(dataPlain, buffer + 5);
		}
		else if (std::memcmp(buffer + 5, "mags\0\0\0\0", 8) == 0)
		{
			const string dataPlain = manager->GetCs2Magnetartikel();
			SendCompressedFile(dataPlain, buffer + 5);
		}
		else if (std::memcmp(buffer + 5, "gbs", 3) == 0
			&& buffer[12] == 0x00) // we need a null here to prevent parsing errors
		{
			if (std::memcmp(buffer + 5, "gbs\0\0\0\0\0", 8) == 0)
			{
				const string dataPlain = manager->GetCs2GBS();
				SendCompressedFile(dataPlain, buffer + 5);
			}
			else if (buffer[8] == '-')
			{
				const string gbsAsString(reinterpret_cast<const char*>(buffer + 9));
				const unsigned int gbs = Utils::Integer::StringToInteger(gbsAsString);
				const string dataPlain = manager->GetCs2GBS(gbs);
				SendCompressedFile(dataPlain, buffer + 5);
			}
		}
		else if (std::memcmp(buffer + 5, "fs\0\0\0\0\0\0", 8) == 0)
		{
			// we send an empty configuration
			const string dataPlain = "[fahrstrassen]\nversion\n .minor=4\n";
			SendCompressedFile(dataPlain, buffer + 5);
		}
	}

	void MaerklinCANCommon::SendCompressedFile(const string& dataPlain, const unsigned char* fileName)
	{
		logger->Debug(dataPlain);
		const uint32_t dataPlainSize = static_cast<uint32_t>(dataPlain.size());
		const string dataCompressed = ZLib::Compress(dataPlain);
		const uint32_t dataCompressedSize = static_cast<uint32_t>(dataCompressed.size());
		const uint32_t dataSize = dataCompressedSize + 4;

		// Keep the historical compressed-file padding, including an extra block
		// when the data size is already a multiple of 8.
		const uint32_t dataToSendSize = (dataSize + 8) & 0xFFFFFFF8;
		vector<unsigned char> dataToSend(dataToSendSize, 0);
		Utils::Integer::IntToDataBigEndian(dataPlainSize, dataToSend.data());
		std::memcpy(dataToSend.data() + 4, dataCompressed.data(), dataCompressedSize);

		SendFile(dataToSend, dataSize, fileName);
	}

	void MaerklinCANCommon::SendUncompressedFile(const string& data,
		const unsigned char* fileName,
		const CanHash destinationHash)
	{
		logger->Debug(data);
		const uint32_t dataSize = static_cast<uint32_t>(data.size());
		const uint32_t dataToSendSize = (dataSize + 7) & 0xFFFFFFF8;
		vector<unsigned char> dataToSend(dataToSendSize, 0);
		std::memcpy(dataToSend.data(), data.data(), dataSize);

		SendFile(dataToSend, dataSize, fileName, &destinationHash);
	}

	void MaerklinCANCommon::SendFile(const vector<unsigned char>& data,
		const uint32_t dataSize,
		const unsigned char* fileName,
		const CanHash* destinationHash)
	{
		unsigned char sendBuffer[CANCommandBufferLength];
		if (fileName)
		{
			// send filename as response
			CreateCommandHeader(sendBuffer, CanCommandRequestConfigData, CanResponseResponse, 8);
			Utils::Utils::Copy8Bytes(fileName, sendBuffer + 5);
			SendInternal(sendBuffer);
		}

		const CanFileCrc crc = CalcCrc(data.data(), data.size());

		// send first data packet
		CreateCommandHeader(sendBuffer, CanCommandConfigData, CanResponseCommand, 6);
		if (destinationHash)
		{
			Utils::Integer::ShortToDataBigEndian(*destinationHash, sendBuffer + 2);
		}
		Utils::Integer::IntToDataBigEndian(dataSize, sendBuffer + 5);
		Utils::Integer::ShortToDataBigEndian(crc, sendBuffer + 9);
		SendInternal(sendBuffer);

		// send following data packets
		CreateCommandHeader(sendBuffer, CanCommandConfigData, CanResponseCommand, 8);
		if (destinationHash)
		{
			Utils::Integer::ShortToDataBigEndian(*destinationHash, sendBuffer + 2);
		}
		for(size_t sent = 0; sent < data.size(); sent += 8)
		{
			Utils::Utils::Copy8Bytes(data.data() + sent, sendBuffer + 5);
			Utils::Utils::SleepForMilliseconds(2); // do not overload CS2 with too much data
			SendInternal(sendBuffer);
		}
	}

	void MaerklinCANCommon::DeleteCanFile(MaerklinCANCommon::CanFile* canFile)
	{
		if (!canFile)
		{
			return;
		}
		if (!canFile->data)
		{
			return;
		}
		free(canFile->data);
		canFiles.erase(canFiles.begin() + (canFile - canFiles.data()));
	}

	struct MaerklinCANCommon::CanFile* MaerklinCANCommon::GetCanFile(const CanHash canHash)
	{

		for (struct CanFile& canFile : canFiles)
		{
			if (canFile.hash != canHash)
			{
				// another canFile, go to next
				continue;
			}

			// canFile found
			return &canFile;
		}

		// create and add canFile to vector
		struct CanFile canFile;
		memset(&canFile, 0, sizeof(struct CanFile));
		canFile.hash = canHash;
		canFiles.push_back(canFile);
		std::vector<struct CanFile>::iterator lastCanFile = canFiles.end() - 1;
		return &(*lastCanFile);
	}

	void MaerklinCANCommon::ParseCommandConfigData(const unsigned char* const buffer)
	{
		const CanHash canHash = ParseHash(buffer);
		struct CanFile* canFile = GetCanFile(canHash);
		const CanLength length = ParseLength(buffer);
		switch (length)
		{
			case 6:
			case 7:
				ParseCommandConfigDataFirst(buffer, canFile);
				return;

			case 8:
				ParseCommandConfigDataNext(buffer, canFile);
				return;

			default:
				return;
		}
		return;
	}

	void MaerklinCANCommon::ParseCommandConfigDataFirst(const unsigned char* const buffer,
		struct CanFile* canFile)
	{
		if (!canFile)
		{
			return;
		}
		if (canFile->data)
		{
			free(canFile->data);
		}
		canFile->dataSize = Utils::Integer::DataBigEndianToInt(buffer + 5);
		canFile->crc = Utils::Integer::DataBigEndianToShort(buffer + 9);
		canFile->crcSize = (canFile->dataSize + 8) & 0xFFFFFFF8;
		canFile->data = reinterpret_cast<unsigned char*>(malloc(canFile->crcSize));
		canFile->dataPointer = canFile->data;
	}

	void MaerklinCANCommon::ParseCommandConfigDataNext(const unsigned char* const buffer,
		struct CanFile* canFile)
	{
		if (!canFile)
		{
			return;
		}
		if (!canFile->data)
		{
			return;
		}

		Utils::Utils::Copy8Bytes(buffer + 5, canFile->dataPointer);
		canFile->dataPointer += 8;
		if (canFile->dataSize > static_cast<size_t>(canFile->dataPointer - canFile->data))
		{
			return;
		}

		const CanFileCrc calculatedCrc = CalcCrc(canFile->data, canFile->crcSize);
		if (canFile->crc != calculatedCrc)
		{
			logger->Info(Languages::TextCrcMissmatch, Utils::Integer::IntegerToHex(canFile->crc), Utils::Integer::IntegerToHex(calculatedCrc));
			DeleteCanFile(canFile);
			return;
		}

		size_t canFileUncompressedSize = Utils::Integer::DataBigEndianToInt(canFile->data);
		logger->Info(Languages::TextConfigFileReceivedWithSize, canFileUncompressedSize);
		string file = ZLib::UnCompress(reinterpret_cast<char*>(canFile->data + 4), canFile->dataSize, canFileUncompressedSize);
		deque<string> lines;
		Utils::Utils::SplitString(file, "\n", lines);
		for (std::string& line : lines)
		{
			logger->Debug(line);
		}

		ParseCs2File(lines);
		DeleteCanFile(canFile);
	}

	void MaerklinCANCommon::ParseResponseS88Event(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 8)
		{
			return;
		}
		const char* onOff;
		DataModel::Feedback::FeedbackState state;
		if (buffer[10])
		{
			onOff = Languages::GetText(Languages::TextOn);
			state = DataModel::Feedback::FeedbackStateOccupied;
		}
		else
		{
			onOff = Languages::GetText(Languages::TextOff);
			state = DataModel::Feedback::FeedbackStateFree;
		}
		FeedbackDevice device;
		FeedbackBus bus;
		FeedbackPin pin;
		ParseFeedbackPin(buffer, device, bus, pin);
		logger->Info(Languages::TextFeedbackChangeCS2, pin & 0x00000007, (pin >> 3) & 0x0000003F, bus, device, onOff);
		manager->FeedbackState(controlID, pin, device, bus, state);
	}

	void MaerklinCANCommon::ParseResponseReadConfig(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 7)
		{
			return;
		}
		const CvNumber cv = Utils::Integer::DataBigEndianToShort(buffer + 9);
		const CvValue value = buffer[11];
		Address address;
		Protocol protocol;
		LocoType type;
		ParseAddressProtocol(buffer, address, protocol, type);
		logger->Debug("XXX {0} sid={1} cv={2} index={3} value={4}", Utils::Utils::ProtocolToString(protocol), address, cv & 0x3FF, cv >> 10, value);
		if ((protocol == ProtocolMFX) && ParseMfxCVValue(address, cv, value))
		{
			return;
		}
		logger->Info(Languages::TextProgramReadMfxValue, cv & 0x3FF, cv >> 10, value);
		manager->ProgramValue(cv, value);
	}

	void MaerklinCANCommon::ParseResponsePing(const unsigned char* const buffer)
	{
		const uint16_t deviceType = Utils::Integer::DataBigEndianToShort(buffer + 11);
		if (deviceType == CanDeviceCs2Main)
		{
			hasCs2Main = true;
		}
		const string hash = Utils::Integer::IntegerToHex(Utils::Integer::DataBigEndianToShort(buffer + 2));
		const unsigned char deviceId = buffer[8];
		const unsigned char majorVersion = buffer[9];
		const unsigned char minorVersion = buffer[10];
		logger->Debug(Languages::TextDeviceOnCanBus, DeviceTypeAsString(deviceType), hash, deviceId, majorVersion, minorVersion);
	}

	bool MaerklinCANCommon::ParseCs2FileKeyValue(const string& line, string& key, string& value)
	{
		if (line.length() < 4 || line[0] != ' ' || line[1] != '.')
		{
			return false;
		}
		const string stripedLine = line.substr(2);
		Utils::Utils::SplitString(stripedLine, "=", key, value);
		return (key.compare(stripedLine) != 0);
	}

	bool MaerklinCANCommon::ParseCs2FileSubkeyValue(const string& line, string& key, string& value)
	{
		if (line.length() < 5 || line[0] != ' ' || line[1] != '.' || line[2] != '.')
		{
			return false;
		}
		const string stripedLine = line.substr(3);
		Utils::Utils::SplitString(stripedLine, "=", key, value);
		return (key.compare(stripedLine) != 0);
	}

	void MaerklinCANCommon::ParseCs2FileLocomotiveFunction(deque<string>& lines, LocoCacheEntry& cacheEntry)
	{
		lines.pop_front();
		DataModel::LocoFunctionNr nr = 0;
		DataModel::LocoFunctionType type = DataModel::LocoFunctionTypeNone;
		DataModel::LocoFunctionIcon icon = DataModel::LocoFunctionIconNone;
		DataModel::LocoFunctionTimer timer = 0;
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileSubkeyValue(line, key, value);
			if (!ok)
			{
				break;
			}
			if (key.compare("nr") == 0)
			{
				nr = Utils::Integer::StringToInteger(value);
			}
			else if (key.compare("typ") == 0 || key.compare("typ2") == 0)
			{
				uint8_t valueInt = Utils::Integer::StringToInteger(value);
				icon = MapLocoFunctionCs2ToRailControl(static_cast<LocoFunctionCs2Icon>(valueInt & 0x7F));
				type = static_cast<DataModel::LocoFunctionType>((valueInt >> 7) + 1); // CS2: 1 = permanent, 2 = once
			}
			else if (key.compare("dauer") == 0 || key.compare("dauer2") == 0)
			{
				type = DataModel::LocoFunctionTypeTimer;
				timer = Utils::Integer::StringToInteger(value);
			}
			lines.pop_front();
		}
		if (type == DataModel::LocoFunctionTypeNone)
		{
			icon = DataModel::LocoFunctionIconNone;
			timer = 0;
			cacheEntry.ClearFunction(nr);
			return;
		}
		cacheEntry.SetFunction(nr, type, icon, timer);
		if (type == DataModel::LocoFunctionTypeTimer)
		{
			logger->Info(Languages::TextCs2MainLocoFunctionIconTypeTimer, nr, icon, timer);
		}
		else
		{
			logger->Info(Languages::TextCs2MainLocoFunctionIconType, nr, icon, type);
		}
	}

	void MaerklinCANCommon::ParseCs2FileLocomotiveTraktion(deque<string>& lines, LocoCacheEntry& cacheEntry)
	{
		lines.pop_front();
		Protocol protocol = ProtocolNone;
		Address address = AddressNone;
		string name;
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileSubkeyValue(line, key, value);
			if (!ok)
			{
				break;
			}
			if (key.compare("lok") == 0)
			{
				Address input = Utils::Integer::HexToInteger(value);
				LocoType type;
				ParseAddressProtocol(input, address, protocol, type);
				logger->Info(Languages::TextCs2MainLocoSlaveProtocolAddress, Utils::Utils::ProtocolToString(protocol), address);
			}
			else if (key.compare("lokname") == 0)
			{
				name = value;
				logger->Info(Languages::TextCs2MainLocoSlaveName, name);
			}
			lines.pop_front();
		}
		cacheEntry.AddSlave(protocol, address, name);
	}

	void MaerklinCANCommon::ParseCs2FileLocomotive(deque<string>& lines)
	{
		lines.pop_front();
		LocoCacheEntry cacheEntry(controlID);
		std::string name;
		std::string oldName;
		bool remove = false;
		while (lines.size())
		{
			string& line = lines.front();
			if ((line.length() == 0) || (line[0] != ' '))
			{
				break;
			}
			string key;
			string value;
			ParseCs2FileKeyValue(line, key, value);
			if (key.compare("name") == 0)
			{
				name = value;
				cacheEntry.SetName(value);
				cacheEntry.SetMatchKey(value);
				logger->Info(Languages::TextCs2MainLocoName, value);
			}
			else if (key.compare("vorname") == 0)
			{
				oldName = value;
				logger->Info(Languages::TextCs2MainLocoOldName, value);
			}
			else if (key.compare("toRemove") == 0)
			{
				remove = true;
			}
			else if (key.compare("uid") == 0)
			{
				Address input = Utils::Integer::HexToInteger(value);
				Address address = AddressNone;
				Protocol protocol = ProtocolNone;
				LocoType type;
				ParseAddressProtocol(input, address, protocol, type);
				cacheEntry.SetAddress(address);
				cacheEntry.SetProtocol(protocol);
				cacheEntry.SetType(type);
				logger->Info(Languages::TextCs2MainLocoProtocolAddress, Utils::Utils::ProtocolToString(protocol), address);
			}
			else if (key.compare("mfxuid") == 0)
			{
				cacheEntry.SetMfxUid(static_cast<CanUid>(Utils::Integer::HexToInteger(value)));
			}
			else if ((key.compare("funktionen") == 0)
				|| (key.compare("funktionen_2") == 0)
				|| (key.compare("fkt") == 0)
				|| (key.compare("fkt2") == 0))
			{
				ParseCs2FileLocomotiveFunction(lines, cacheEntry);
				continue;
			}
			else if (key.compare("traktion") == 0)
			{
				ParseCs2FileLocomotiveTraktion(lines, cacheEntry);
				continue;
			}
			lines.pop_front();
		}

		if (remove)
		{
			logger->Info(Languages::TextCs2MainLocoRemove, name);
			LocoID locoId = CacheDelete(name);
			manager->LocoDelete(locoId);
		}
		else if (oldName.size() > 0)
		{
			CacheSave(cacheEntry, oldName);
		}
		else
		{
			CacheSave(cacheEntry);
		}
	}

	void MaerklinCANCommon::ParseCs2FileLocomotivesSession(deque<string>& lines)
	{
		lines.pop_front();
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileKeyValue(line, key, value);
			if (!ok)
			{
				return;
			}
			// we do not parse any data in session
			lines.pop_front();
		}
	}

	void MaerklinCANCommon::ParseCs2FileLocomotivesVersion(deque<string>& lines)
	{
		lines.pop_front();
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileKeyValue(line, key, value);
			if (!ok)
			{
				return;
			}
			if ((key.compare("minor") == 0) && (value.compare("3") != 0) && (value.compare("4") != 0))
			{
				logger->Warning(Languages::TextCs2MinorVersionIsUnknown);
			}
			lines.pop_front();
		}
	}

	void MaerklinCANCommon::ParseCs2FileLocomotives(deque<string>& lines)
	{
		lines.pop_front();
		while (lines.size())
		{
			string& line = lines.front();
			if (line.length() == 0)
			{
				return;
			}
			if (line.compare("version") == 0)
			{
				ParseCs2FileLocomotivesVersion(lines);
				continue;
			}
			else if (line.compare("session") == 0)
			{
				ParseCs2FileLocomotivesSession(lines);
				continue;
			}
			else if (line.compare("lokomotive") == 0)
			{
				ParseCs2FileLocomotive(lines);
				continue;
			}
			return;
		}
	}

	void MaerklinCANCommon::ParseCs2File(deque<string>& lines)
	{
		while (lines.size())
		{
			string& line = lines.front();
			if (line.length() == 0 || line[0] != '[')
			{
				return;
			}
			if (line.compare("[lokomotive]") == 0)
			{
				ParseCs2FileLocomotives(lines);
				CacheUpdateSlaves();
				continue;
			}
			return;
		}
	}

	const DataModel::LocoFunctionIcon MaerklinCANCommon::LocoFunctionMapCs2ToRailControl[MaxNrOfCs2FunctionIcons] =
	{
		DataModel::LocoFunctionIconNone,
		DataModel::LocoFunctionIconLight,
		DataModel::LocoFunctionIconInteriorLight1,
		DataModel::LocoFunctionIconBacklightForward,
		DataModel::LocoFunctionIconHeadlightHighBeamForward,
		DataModel::LocoFunctionIconSoundGeneral,
		DataModel::LocoFunctionIconPanto12,
		DataModel::LocoFunctionIconSmokeGenerator,
		DataModel::LocoFunctionIconShuntingMode,
		DataModel::LocoFunctionIconTelex12,
		DataModel::LocoFunctionIconHorn1,
		DataModel::LocoFunctionIconWhistle1,
		DataModel::LocoFunctionIconWhistle2,
		DataModel::LocoFunctionIconBell,
		DataModel::LocoFunctionIconLeftRight,
		DataModel::LocoFunctionIconUpDown1,
		DataModel::LocoFunctionIconTurnLeft,
		DataModel::LocoFunctionIconUpDown2,
		DataModel::LocoFunctionIconInertia,
		DataModel::LocoFunctionIconFan2,
		DataModel::LocoFunctionIconBreak1,
		DataModel::LocoFunctionIconGearBox,
		DataModel::LocoFunctionIconGenerator,
		DataModel::LocoFunctionIconRunning1,
		DataModel::LocoFunctionIconEngine1,
		DataModel::LocoFunctionIconStationAnnouncement1,
		DataModel::LocoFunctionIconShovelCoal,
		DataModel::LocoFunctionIconCloseDoor,
		DataModel::LocoFunctionIconOpenDoor,
		DataModel::LocoFunctionIconFan1,
		DataModel::LocoFunctionIconFan,
		DataModel::LocoFunctionIconFireBox,
		DataModel::LocoFunctionIconInteriorLight2,
		DataModel::LocoFunctionIconTableLight3,
		DataModel::LocoFunctionIconTableLight2,
		DataModel::LocoFunctionIconTableLight1,
		DataModel::LocoFunctionIconShakingRust,
		DataModel::LocoFunctionIconRailJoint,
		DataModel::LocoFunctionIconLocomotiveNumberIndicator,
		DataModel::LocoFunctionIconMusic1,
		DataModel::LocoFunctionIconTrainDestinationIndicator,
		DataModel::LocoFunctionIconCabLight2,
		DataModel::LocoFunctionIconCabLight1,
		DataModel::LocoFunctionIconCoupler,
		DataModel::LocoFunctionIconBufferPush,
		DataModel::LocoFunctionIconStationAnnouncement3,
		DataModel::LocoFunctionIconCraneHook,
		DataModel::LocoFunctionIconBlinkingLight,
		DataModel::LocoFunctionIconCabLight12,
		DataModel::LocoFunctionIconCompressedAir,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconTelex2,
		DataModel::LocoFunctionIconTelex1,
		DataModel::LocoFunctionIconPanto2,
		DataModel::LocoFunctionIconPanto1,
		DataModel::LocoFunctionIconHeadlightLowBeamReverse,
		DataModel::LocoFunctionIconHeadlightLowBeamForward,
		DataModel::LocoFunctionIconUp,
		DataModel::LocoFunctionIconFan3,
		DataModel::LocoFunctionIconEngineLight,
		DataModel::LocoFunctionIconSteamBlowOut,
		DataModel::LocoFunctionIconSteamBlow,
		DataModel::LocoFunctionIconCrane,
		DataModel::LocoFunctionIconUp,
		DataModel::LocoFunctionIconDown,
		DataModel::LocoFunctionIconLeft,
		DataModel::LocoFunctionIconRight,
		DataModel::LocoFunctionIconTurnRight,
		DataModel::LocoFunctionIconMagnet,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconPanto,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconRadio,
		DataModel::LocoFunctionIconStationAnnouncement2,
		DataModel::LocoFunctionIconBacklightReverse,
		DataModel::LocoFunctionIconAirPump,
		DataModel::LocoFunctionIconSpeak,
		DataModel::LocoFunctionIconEngine2,
		DataModel::LocoFunctionIconNoSound,
		DataModel::LocoFunctionIconStairsLight,
		DataModel::LocoFunctionIconFillWater,
		DataModel::LocoFunctionIconBreak2,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault,
		DataModel::LocoFunctionIconDefault
	};

	const MaerklinCANCommon::LocoFunctionCs2Icon MaerklinCANCommon::LocoFunctionMapRailControlToCs2[DataModel::MaxLocoFunctionIcons] =
	{
		LocoFunctionCs2IconNone,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconShuntingMode,
		LocoFunctionCs2IconInertia,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconLight,
		LocoFunctionCs2IconLight,
		LocoFunctionCs2IconLight,
		LocoFunctionCs2IconHeadlightHighBeamForward,
		LocoFunctionCs2IconLight,
		LocoFunctionCs2IconHeadlightForward,
		LocoFunctionCs2IconHeadlightReverse,
		LocoFunctionCs2IconBackLightForward,
		LocoFunctionCs2IconBackLightReverse,
		LocoFunctionCs2IconLight,
		LocoFunctionCs2IconBlinkingLight,
		LocoFunctionCs2IconInteriorLight1,
		LocoFunctionCs2IconInteriorLight2,
		LocoFunctionCs2IconTableLight1,
		LocoFunctionCs2IconTableLight2,
		LocoFunctionCs2IconTableLight3,
		LocoFunctionCs2IconCabLight1,
		LocoFunctionCs2IconCabLight2,
		LocoFunctionCs2IconCabLight12,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconTrainDestinationIndicator,
		LocoFunctionCs2IconLocomotiveNumberIndicator,
		LocoFunctionCs2IconEngineLight,
		LocoFunctionCs2IconFireBox,
		LocoFunctionCs2IconStairsLight,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconSmokeGenerator,
		LocoFunctionCs2IconTelex1,
		LocoFunctionCs2IconTelex2,
		LocoFunctionCs2IconTelex12,
		LocoFunctionCs2IconPanto1,
		LocoFunctionCs2IconPanto2,
		LocoFunctionCs2IconPanto12,
		LocoFunctionCs2IconUp1,
		LocoFunctionCs2IconDown,
		LocoFunctionCs2IconUpDown1,
		LocoFunctionCs2IconUpDown2,
		LocoFunctionCs2IconLeft,
		LocoFunctionCs2IconRight,
		LocoFunctionCs2IconLeftRight,
		LocoFunctionCs2IconTurnLeft,
		LocoFunctionCs2IconTurnRight,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconCrane,
		LocoFunctionCs2IconMagnet,
		LocoFunctionCs2IconCraneHook,
		LocoFunctionCs2IconFan,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconDefault,
		LocoFunctionCs2IconNoSound,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconRunning1,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconEngine1,
		LocoFunctionCs2IconEngine2,
		LocoFunctionCs2IconBreak1,
		LocoFunctionCs2IconBreak2,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconHorn1,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconWhistle1,
		LocoFunctionCs2IconWhistle2,
		LocoFunctionCs2IconBell,
		LocoFunctionCs2IconStationAnnouncement1,
		LocoFunctionCs2IconStationAnnouncement2,
		LocoFunctionCs2IconStationAnnouncement3,
		LocoFunctionCs2IconSpeak,
		LocoFunctionCs2IconRadio,
		LocoFunctionCs2IconMusic1,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconOpenDoor,
		LocoFunctionCs2IconCloseDoor,
		LocoFunctionCs2IconFan1,
		LocoFunctionCs2IconFan2,
		LocoFunctionCs2IconFan3,
		LocoFunctionCs2IconShovelCoal,
		LocoFunctionCs2IconCompressedAir,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconSteamBlowOut,
		LocoFunctionCs2IconSteamBlow,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconShakingRust,
		LocoFunctionCs2IconAirPump,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconBufferPush,
		LocoFunctionCs2IconGenerator,
		LocoFunctionCs2IconGearBox,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconFillWater,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconRailJoint,
		LocoFunctionCs2IconCoupler,
		LocoFunctionCs2IconPanto,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconSoundGeneral,
		LocoFunctionCs2IconNoSound,
		LocoFunctionCs2IconNoSound
	};
}} // namespace

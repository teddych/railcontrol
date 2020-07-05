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

#include <deque>

#include "Hardware/ProtocolMaerklinCAN.h"
#include "Hardware/ZLib.h"

using std::deque;
using std::string;
using std::vector;

namespace Hardware
{
	void ProtocolMaerklinCAN::Init()
	{
		receiverThread = std::thread(&ProtocolMaerklinCAN::Receiver, this);
		cs2MasterThread = std::thread(&ProtocolMaerklinCAN::Cs2MasterThread, this);
	}

	ProtocolMaerklinCAN::~ProtocolMaerklinCAN()
	{
		if (canFileData != nullptr)
		{
			free(canFileData);
			canFileData = nullptr;
		}
		if (run == false)
		{
			return;
		}
		run = false;
		receiverThread.join();
		cs2MasterThread.join();
	}

	void ProtocolMaerklinCAN::Wait(const unsigned int duration) const
	{
		unsigned int wait = duration;
		while (run && !hasCs2Master && wait)
		{
			Utils::Utils::SleepForSeconds(1);
			--wait;
		}
	}

	void ProtocolMaerklinCAN::Cs2MasterThread()
	{
		run = true;
		Wait(30);

		while (run && !hasCs2Master)
		{
			Ping();
			Wait(10);
		}
		if (hasCs2Master)
		{
			RequestLoks();
		}
	}

	void ProtocolMaerklinCAN::CreateCommandHeader(unsigned char* const buffer, const CanCommand command, const CanResponse response, const CanLength length)
	{
		const CanPrio prio = 0;
		buffer[0] = (prio << 1) | (command >> 7);
		buffer[1] = (command << 1) | (response & 0x01);
		Utils::Utils::ShortToDataBigEndian(hash, buffer + 2);
		buffer[4] = length;
		int64_t* data = (int64_t*) (buffer + 5);
		*data = 0L;
	}

	void ProtocolMaerklinCAN::ParseAddressProtocol(const Address input, Address& address, Protocol& protocol)
	{
		address = input;
		Address maskedAddress = address & 0xFC00;

		if (maskedAddress == 0x0000 || maskedAddress == 0x3000)
		{
			protocol = ProtocolMM;
			address &= 0x03FF;
			return;
		}

		if (maskedAddress == 0x3800 || maskedAddress == 0x3C00)
		{
			protocol = ProtocolDCC;
			address &= 0x03FF;
			return;
		}

		maskedAddress = address & 0xC000;
		address &= 0x3FFF;
		if (maskedAddress == 0x4000)
		{
			protocol = ProtocolMFX;
			return;
		}
		if (maskedAddress == 0xC000)
		{
			protocol = ProtocolDCC;
			return;
		}

		protocol = ProtocolNone;
		address = 0;
	}

	ProtocolMaerklinCAN::CanHash ProtocolMaerklinCAN::CalcHash(const CanUid uid)
	{
		CanHash calc = (uid >> 16) ^ (uid & 0xFFFF);
		CanHash out = ((calc << 3) | 0x0300) & 0xFF00;
		out |= (calc & 0x007F);
		return out;
	}

	void ProtocolMaerklinCAN::GenerateUidHash()
	{
		uid = Utils::Utils::RandInt();
		string uidString = Utils::Utils::IntegerToHex(uid);
		params->SetArg5(uidString);
		hash = CalcHash(uid);
		logger->Info(Languages::TextMyUidHash, uidString, Utils::Utils::IntegerToHex(hash));
	}

	void ProtocolMaerklinCAN::CreateLocalIDLoco(unsigned char* buffer, const Protocol& protocol, const Address& address)
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
		// else expect PROTOCOL_MM2: do nothing
		Utils::Utils::IntToDataBigEndian(localID, buffer + 5);
	}

	void ProtocolMaerklinCAN::CreateLocalIDAccessory(unsigned char* buffer, const Protocol& protocol, const Address& address)
	{
		uint32_t localID = address - 1; // GUI-address is 1-based, protocol-address is 0-based
		if (protocol == ProtocolDCC)
		{
			localID |= 0x3800;
		}
		else
		{
			localID |= 0x3000;
		}
		Utils::Utils::IntToDataBigEndian(localID, buffer + 5);
	}

	void ProtocolMaerklinCAN::Booster(const BoosterState status)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(status ? Languages::TextTurningBoosterOn : Languages::TextTurningBoosterOff);
		CreateCommandHeader(buffer, CanCommandSystem, CanResponseCommand, 5);
		buffer[9] = status;
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::LocoSpeed(const Protocol protocol, const Address address, const Speed speed)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingSpeedWithProtocol, protocol, address, speed);
		CreateCommandHeader(buffer, CanCommandLocoSpeed, CanResponseCommand, 6);
		CreateLocalIDLoco(buffer, protocol, address);
		Utils::Utils::ShortToDataBigEndian(speed, buffer + 9);
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::LocoOrientation(const Protocol protocol, const Address address, const Orientation orientation)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingDirectionOfTravelWithProtocol, protocol, address, Languages::GetLeftRight(orientation));
		CreateCommandHeader(buffer, CanCommandLocoDirection, CanResponseCommand, 5);
		CreateLocalIDLoco(buffer, protocol, address);
		buffer[9] = (orientation ? 1 : 2);
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::LocoFunction(const Protocol protocol, const Address address, const Function function, const DataModel::LocoFunctions::FunctionState on)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingFunctionWithProtocol, static_cast<int>(function), static_cast<int>(protocol), address, Languages::GetOnOff(on));
		CreateCommandHeader(buffer, CanCommandLocoFunction, CanResponseCommand, 6);
		CreateLocalIDLoco(buffer, protocol, address);
		buffer[9] = function;
		buffer[10] = (on == DataModel::LocoFunctions::FunctionStateOn);
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::AccessoryOnOrOff(const Protocol protocol, const Address address, const DataModel::AccessoryState state, const bool on)
	{
		unsigned char buffer[CANCommandBufferLength];
		logger->Info(Languages::TextSettingAccessoryWithProtocol, static_cast<int>(protocol), address, Languages::GetGreenRed(state), Languages::GetOnOff(on));
		CreateCommandHeader(buffer, CanCommandAccessory, CanResponseCommand, 6);
		CreateLocalIDAccessory(buffer, protocol, address);
		buffer[9] = state & 0x03;
		buffer[10] = static_cast<unsigned char>(on);
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::ProgramRead(const ProgramMode mode, const Address address, const CvNumber cv)
	{
		Address addressInternal = address;
		Protocol protocol = ProtocolNone;
		switch (mode)
		{
			case ProgramModeDccDirect:
				logger->Info(Languages::TextProgramDccRead, cv);
				protocol = ProtocolDCC;
				break;

			case ProgramModeMfx:
				logger->Info(Languages::TextProgramMfxRead, address, cv);
				protocol = ProtocolMFX;
				break;

			default:
				return;
		}
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandReadConfig, CanResponseCommand, 7);
		CreateLocalIDLoco(buffer, protocol, addressInternal);
		Utils::Utils::ShortToDataBigEndian(cv, buffer + 9);
		buffer[11] = 1;
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::ProgramWrite(const ProgramMode mode, const Address address, const CvNumber cv, const CvValue value)
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
				logger->Info(Languages::TextProgramDccWrite, cv, value);
				protocol = ProtocolDCC;
				addressInternal = 0;
				break;

			case ProgramModeDccPomLoco:
				logger->Info(Languages::TextProgramDccWrite, address, cv, value);
				protocol = ProtocolDCC;
				controlFlags = 1 << 7;
				break;

			case ProgramModeMfx:
				logger->Info(Languages::TextProgramMfxWrite, address, cv, value);
				protocol = ProtocolMFX;
				controlFlags = 1 << 7;
				break;

			default:
				return;
		}
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandWriteConfig, CanResponseCommand, 8);
		CreateLocalIDLoco(buffer, protocol, addressInternal);
		Utils::Utils::ShortToDataBigEndian(cv, buffer + 9);
		buffer[11] = value;
		buffer[12] = controlFlags;
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::Ping()
	{
		unsigned char buffer[CANCommandBufferLength];
		CreateCommandHeader(buffer, CanCommandPing, CanResponseCommand, 0);
		SendInternal(buffer);
	}

	void ProtocolMaerklinCAN::RequestLoks()
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

	void ProtocolMaerklinCAN::Parse(const unsigned char* buffer)
	{
		CanResponse response = ParseResponse(buffer);
		CanCommand command = ParseCommand(buffer);
		CanLength length = ParseLength(buffer);
		logger->Hex(buffer, 5 + length);
		const CanHash receivedHash = ParseHash(buffer);
		if (receivedHash == hash)
		{
			uint16_t deviceType = Utils::Utils::DataBigEndianToShort(buffer + 11);
			if (command == CanCommandPing && response == true)
			{
				if (deviceType == CanDeviceCs2Master)
				{
					hasCs2Master = true;
					logger->Info(Languages::TextCs2MasterFound);
				}
			}
			else if (command != CanCommandConfigData)
			{
				GenerateUidHash();
			}
		}
		if (response == true)
		{
			switch (command)
			{
				case CanCommandS88Event:
					ParseResponseS88Event(buffer);
					return;

				case CanCommandReadConfig:
					ParseResponseReadConfig(buffer);
					return;

				case CanCommandPing:
					ParseResponsePing(buffer);
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

			case CanCommandLocoSpeed:
				ParseCommandLocoSpeed(buffer);
				return;

			case CanCommandLocoDirection:
				ParseCommandLocoDirection(buffer);
				return;

			case CanCommandLocoFunction:
				ParseCommandLocoFunction(buffer);
				return;

			case CanCommandAccessory:
				ParseCommandAccessory(buffer);
				return;

			case CanCommandPing:
				ParseCommandPing(buffer);
				return;

			case CanCommandConfigData:
				ParseCommandConfigData(buffer);
				return;

			default:
				return;
		}
	}

	void ProtocolMaerklinCAN::ParseCommandSystem(const unsigned char* const buffer)
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
		}
	}

	void ProtocolMaerklinCAN::ParseCommandLocoSpeed(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 6)
		{
			return;
		}
		Address address;
		Protocol protocol;
		ParseAddressProtocol(buffer, address, protocol);
		Speed speed = Utils::Utils::DataBigEndianToShort(buffer + 9);
		logger->Info(Languages::TextReceivedSpeedCommand, protocol, address, speed);
		manager->LocoSpeed(ControlTypeHardware, controlID, protocol, address, speed);
	}

	void ProtocolMaerklinCAN::ParseCommandLocoDirection(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 5)
		{
			return;
		}
		Address address;
		Protocol protocol;
		ParseAddressProtocol(buffer, address, protocol);
		Orientation orientation = (buffer[9] == 1 ? OrientationRight : OrientationLeft);
		logger->Info(Languages::TextReceivedDirectionCommand, protocol, address, orientation);
		// changing direction implies speed = 0
		manager->LocoSpeed(ControlTypeHardware, controlID, protocol, address, MinSpeed);
		manager->LocoOrientation(ControlTypeHardware, controlID, protocol, address, orientation);
	}

	void ProtocolMaerklinCAN::ParseCommandLocoFunction(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 6)
		{
			return;
		}
		Address address;
		Protocol protocol;
		ParseAddressProtocol(buffer, address, protocol);
		Function function = buffer[9];
		DataModel::LocoFunctions::FunctionState on = (buffer[10] != 0 ? DataModel::LocoFunctions::FunctionStateOn : DataModel::LocoFunctions::FunctionStateOff);
		logger->Info(Languages::TextReceivedFunctionCommand, protocol, address, function, on);
		manager->LocoFunction(ControlTypeHardware, controlID, protocol, address, function, on);
	}

	void ProtocolMaerklinCAN::ParseCommandAccessory(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 6 || buffer[10] != 1)
		{
			return;
		}
		Address address;
		Protocol protocol;
		ParseAddressProtocol(buffer, address, protocol);
		DataModel::AccessoryState state = (buffer[9] ? DataModel::AccessoryStateOn : DataModel::AccessoryStateOff);
		// GUI-address is 1-based, protocol-address is 0-based
		++address;
		logger->Info(Languages::TextReceivedAccessoryCommand, protocol, address, state);
		manager->AccessoryState(ControlTypeHardware, controlID, protocol, address, state);
	}

	void ProtocolMaerklinCAN::ParseCommandPing(const unsigned char* const buffer)
	{
		if (buffer[4] == 8 && Utils::Utils::DataBigEndianToInt(buffer + 5) != uid)
		{
			return;
		}
		unsigned char sendBuffer[CANCommandBufferLength];
		CreateCommandHeader(sendBuffer, CanCommandPing, CanResponseResponse, 8);
		Utils::Utils::IntToDataBigEndian(uid, sendBuffer + 5);
		// version 3.8
		sendBuffer[9] = 3;
		sendBuffer[10] = 8;
		// device type CS2 Slave
		sendBuffer[11] = 0xff;
		sendBuffer[12] = 0xf0;
		SendInternal(sendBuffer);
	}

	void ProtocolMaerklinCAN::ParseCommandConfigData(const unsigned char* const buffer)
	{
		CanLength length = ParseLength(buffer);
		switch (length)
		{
			case 6:
			case 7:
				ParseCommandConfigDataFirst(buffer);
				return;

			case 8:
				ParseCommandConfigDataNext(buffer);
				return;

			default:
				return;
		}
		return;
	}

	void ProtocolMaerklinCAN::ParseCommandConfigDataFirst(const unsigned char* const buffer)
	{
		if (canFileData != nullptr)
		{
			free(canFileData);
		}
		canFileDataSize = Utils::Utils::DataBigEndianToInt(buffer + 5);
		canFileCrc = Utils::Utils::DataBigEndianToShort(buffer + 9);
		canFileData = reinterpret_cast<unsigned char*>(malloc(canFileDataSize + 8));
		canFileDataPointer = canFileData;
	}

	void ProtocolMaerklinCAN::ParseCommandConfigDataNext(const unsigned char* const buffer)
	{
		if (canFileData == nullptr)
		{
			return;
		}

		Utils::Utils::Copy8Bytes(buffer + 5, canFileDataPointer);
		canFileDataPointer += 8;
		if (canFileDataSize > static_cast<size_t>(canFileDataPointer - canFileData))
		{
			return;
		}

		size_t canFileUncompressedSize = Utils::Utils::DataBigEndianToInt(canFileData);
		logger->Info(Languages::TextConfigFileReceivedWithSize, canFileUncompressedSize);
		string file = ZLib::UnCompress(reinterpret_cast<char*>(canFileData + 4), canFileDataSize, canFileUncompressedSize);
		deque<string> lines;
		Utils::Utils::SplitString(file, "\n", lines);
		ParseCs2File(lines);
		for (string line : lines)
		{
			logger->Hex(reinterpret_cast<const unsigned char*>(line.c_str()), line.size());
		}

		free(canFileData);
	 	canFileDataSize = 0;
	 	canFileData = nullptr;
	 	canFileDataPointer = nullptr;
	}

	void ProtocolMaerklinCAN::ParseResponseS88Event(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 8)
		{
			return;
		}
		const char *onOff;
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
		Address address = ParseAddress(buffer);
		logger->Info(Languages::TextFeedbackChange, address & 0x000F, address >> 4, onOff);
		manager->FeedbackState(controlID, address, state);
	}

	void ProtocolMaerklinCAN::ParseResponseReadConfig(const unsigned char* const buffer)
	{
		if (ParseLength(buffer) != 7)
		{
			return;
		}
		CvNumber cv = Utils::Utils::DataBigEndianToShort(buffer + 9);
		CvValue value = buffer[11];
		logger->Info(Languages::TextProgramReadValue, cv, value);
		manager->ProgramValue(cv, value);
	}

	void ProtocolMaerklinCAN::ParseResponsePing(const unsigned char* const buffer)
	{
		const uint16_t deviceType = Utils::Utils::DataBigEndianToShort(buffer + 11);
		char* deviceString = nullptr;
		switch (deviceType)
		{
			case CanDeviceGfp:
				deviceString = const_cast<char*>("Gleisformat Prozessor");
				hasCs2Master = true;
				break;

			case CanDeviceGleisbox:
				deviceString = const_cast<char*>("Gleisbox");
				break;

			case CanDeviceConnect6021:
				deviceString = const_cast<char*>("Connect 6021");
				break;

			case CanDeviceMs2:
			case CanDeviceMs2_2:
				deviceString = const_cast<char*>("MS2");
				break;

			case CanDeviceWireless:
				deviceString = const_cast<char*>("Wireless");
				break;

			case CanDeviceCs2Master:
				deviceString = const_cast<char*>("CS2 Master");
				break;

			case CanDeviceCs2Slave:
			case CanDeviceCs2Slave_2:
				deviceString = const_cast<char*>("CS2 Slave");
				break;

			default:
				deviceString = const_cast<char*>("unknown");
				break;
		}
		const string hash = Utils::Utils::IntegerToHex(Utils::Utils::DataBigEndianToShort(buffer + 2));
		const unsigned char majorVersion = buffer[9];
		const unsigned char minorVersion = buffer[10];
		logger->Info(Languages::TextDeviceOnCanBus, deviceString, hash, majorVersion, minorVersion);
	}

	bool ProtocolMaerklinCAN::ParseCs2FileKeyValue(const string& line, string& key, string& value)
	{
		if (line.length() < 4 || line[0] != ' ' || line[1] != '.')
		{
			return false;
		}
		const string stripedLine = line.substr(2);
		Utils::Utils::SplitString(stripedLine, "=", key, value);
		return (key.compare(stripedLine) != 0);
	}

	bool ProtocolMaerklinCAN::ParseCs2FileSubkeyValue(const string& line, string& key, string& value)
	{
		if (line.length() < 5 || line[0] != ' ' || line[1] != '.' || line[2] != '.')
		{
			return false;
		}
		const string stripedLine = line.substr(3);
		Utils::Utils::SplitString(stripedLine, "=", key, value);
		return (key.compare(stripedLine) != 0);
	}

	void ProtocolMaerklinCAN::ParseCs2FileLocomotiveFunction(deque<string>& lines)
	{
		lines.pop_front();
		int nr = 0;
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileSubkeyValue(line, key, value);
			if (ok == false)
			{
				return;
			}
			if (key.compare("nr") == 0)
			{
				nr = Utils::Utils::StringToInteger(value);
			}
			else if (key.compare("typ") == 0)
			{
				std::cout << " Funktion: " << nr << " Typ: " << value << std::endl;
			}
			lines.pop_front();
		}
	}

	void ProtocolMaerklinCAN::ParseCs2FileLocomotive(deque<string>& lines)
	{
		lines.pop_front();
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			if (line.length() == 0 || line[0] != ' ')
			{
				return;
			}
			ParseCs2FileKeyValue(line, key, value);
			if (key.compare("name") == 0)
			{
				logger->Info(Languages::TextCs2MasterLocoName, value);
			}
			if (key.compare("vorname") == 0)
			{
				logger->Info(Languages::TextCs2MasterLocoOldName, value);
			}
			else if (key.compare("uid") == 0)
			{
				Address input = Utils::Utils::HexToInteger(value);
				Address address = AddressNone;
				Protocol protocol = ProtocolNone;
				ParseAddressProtocol(input, address, protocol);
				logger->Info(Languages::TextCs2MasterLocoAddressProtocol, address, protocol);
				std::cout << "Address: " << address << std::endl << "Protocol: " << protocol << std::endl;
			}
			else if (key.compare("funktionen") == 0)
			{
				ParseCs2FileLocomotiveFunction(lines);
				continue;
			}
			lines.pop_front();
		}
	}

	void ProtocolMaerklinCAN::ParseCs2FileLocomotivesSession(deque<string>& lines)
	{
		lines.pop_front();
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileKeyValue(line, key, value);
			if (ok == false)
			{
				return;
			}
			if (key.compare("id") == 0)
			{
				std::cout << "Session ID: " << value << std::endl;
			}
			lines.pop_front();
		}
	}

	void ProtocolMaerklinCAN::ParseCs2FileLocomotivesVersion(deque<string>& lines)
	{
		lines.pop_front();
		while (lines.size())
		{
			string& line = lines.front();
			string key;
			string value;
			bool ok = ParseCs2FileKeyValue(line, key, value);
			if (ok == false)
			{
				return;
			}
			if (key.compare("minor") == 0)
			{
				std::cout << "Minor version: " << value << std::endl;
			}
			lines.pop_front();
		}
	}

	void ProtocolMaerklinCAN::ParseCs2FileLocomotives(deque<string>& lines)
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

	void ProtocolMaerklinCAN::ParseCs2File(deque<string>& lines)
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
				continue;
			}
			return;
		}
	}
} // namespace

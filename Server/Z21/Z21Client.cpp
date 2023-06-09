/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2023 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <cstring>		//memset

#include "Manager.h"
#include "Server/Z21/Z21Client.h"

namespace Z21Enums = Hardware::Protocols::Z21Enums;

namespace Server { namespace Z21
{
	Z21Client::Z21Client(const unsigned int id,
		Manager& manager,
		const int serverSocket,
		const struct sockaddr_storage* clientAddress)
	:	Network::UdpClient(serverSocket, clientAddress, 60),
		logger(Logger::Logger::GetLogger("Z21Client # " + std::to_string(id))),
		manager(manager),
		broadCastFlags(Z21Enums::BroadCastFlagNone)
	{
		logger->Debug(Languages::TextUdpConnectionEstablished, AddressAsString());
		SendSystemStatusChanged();
	}

	Z21Client::~Z21Client()
	{
		logger->Debug(Languages::TextUdpConnectionClosed, AddressAsString());
	}

	void Z21Client::Work(const unsigned char* buffer, const size_t dataLength)
	{
		logger->Hex(buffer, dataLength);

		size_t dataRead = 0;
		while (dataRead < dataLength)
		{
			size_t ret = ParseData(buffer + dataRead, dataLength - dataRead);
			if (ret == 0)
			{
				break;
			}
			dataRead += ret;
		}
	}

	size_t Z21Client::ParseData(const unsigned char* buffer, const size_t bufferLength)
	{
		unsigned short dataLength = Utils::Utils::DataLittleEndianToShort(buffer);
		if (dataLength < 4 || dataLength > bufferLength)
		{
			return 1472; // 1472 = Max UDP data size
		}

		const uint16_t header = Utils::Utils::DataLittleEndianToShort(buffer + 2);
		switch (header)
		{
			case Z21Enums::HeaderLogOff:
				Terminate();
				break;

			case Z21Enums::HeaderSeeXHeader:
			{
				ParseXHeader(buffer);
				break;
			}

			case Z21Enums::HeaderSetBroadcastFlags:
			{
				broadCastFlags = static_cast<Z21Enums::BroadCastFlags>(Utils::Utils::DataLittleEndianToInt(buffer + 4));
				break;
			}

			case Z21Enums::HeaderGetSystemState:
				SendSystemStatusChanged();
				break;

			default:
				// not implemented
				break;
		}
		return dataLength;
	}

	void Z21Client::ParseXHeader(const unsigned char* buffer)
	{
		Z21Enums::XHeader xHeader = static_cast<Z21Enums::XHeader>(buffer[4]);
		switch (xHeader)
		{
			case Z21Enums::XHeaderSeeDB0_1:
				ParseDB0(buffer);
				break;

			case Z21Enums::XHeaderSetStop:
				manager.Booster(ControlTypeZ21Server, BoosterStateStop);
				SendBcStopped();
				break;

			case Z21Enums::XHeaderGetFirmwareVersion:
				SendFirmwareVersion();
				break;

			default:
				break;
		}
	}

	void Z21Client::ParseDB0(const unsigned char* buffer)
	{
		switch (buffer[5])
		{
			case Z21Enums::DB0Status:
			{
				if (buffer[6] != 0x05)
				{
					logger->Error(Languages::TextCheckSumError);
				}
				SendStatusChanged();
				break;
			}

			case Z21Enums::DB0SetPowerOff:
				logger->Debug(Languages::TextBoosterIsTurnedOff);
				manager.Booster(ControlTypeHardware, BoosterStateStop);
				break;

			case Z21Enums::DB0SetPowerOn:
				logger->Debug(Languages::TextBoosterIsTurnedOn);
				manager.Booster(ControlTypeHardware, BoosterStateGo);
				break;

			default:
				break;
		}
	}
}} // namespace Server::Z21

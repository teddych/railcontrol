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

#pragma once

#include <ctime>

#include "Hardware/Protocols/Z21DataTypes.h"
#include "Network/UdpClient.h"

namespace Z21Enums = Hardware::Protocols::Z21Enums;

namespace Server { namespace Z21
{
	class Z21Server;

	class Z21Client : protected Network::UdpClient
	{
		public:
			Z21Client() = delete;
			Z21Client(const Z21Client&) = delete;
			Z21Client& operator=(const Z21Client&) = delete;

			Z21Client(Manager& manager,
				Logger::Logger* logger,
				const int serverSocket,
				const struct sockaddr_storage* clientAddress);

			~Z21Client();

			void Work(const unsigned char* buffer, const size_t size) override;

			size_t ParseData(const unsigned char* buffer, const size_t bufferLength);

			void ParseXHeader(const unsigned char* buffer);

			void ParseDB0(const unsigned char* buffer);

			inline void SendPowerOff()
			{
				const unsigned char sendBuffer[7] = { 0x07, 0x00, 0x40, 0x00, 0x61, 0x00, 0x61 };
				Send(sendBuffer, sizeof(sendBuffer));
			}

			inline void SendPowerOn()
			{
				const unsigned char sendBuffer[7] = { 0x07, 0x00, 0x40, 0x00, 0x61, 0x01, 0x60 };
				Send(sendBuffer, sizeof(sendBuffer));
			}

		private:
			inline void SendStatusChanged()
			{
				const unsigned char sendBuffer[8] = { 0x08, 0x00, 0x40, 0x00, 0x62, 0x22, 0x00, 0x08 };
				Send(sendBuffer, sizeof(sendBuffer));
			}

			inline void SendSystemStatusChanged()
			{
				const unsigned char sendBuffer[20] = { 0x14, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x20, 0x4E, 0x50, 0x46, 0x00, 0x00, 0x00, 0x73 };
				Send(sendBuffer, sizeof(sendBuffer));
			}

			inline void SendBcStopped()
			{
				const unsigned char sendBuffer[7] = { 0x07, 0x00, 0x40, 0x00, 0x81, 0x00, 0x81 };
				Send(sendBuffer, sizeof(sendBuffer));
			}

			inline void SendFirmwareVersion()
			{
				const unsigned char sendBuffer[9] = { 0x09, 0x00, 0x40, 0x00, 0xF3, 0x0A, 0x01, 0x23, 0xDB };
				Send(sendBuffer, sizeof(sendBuffer));
			}

			Manager& manager;
			Z21Enums::BroadCastFlags broadCastFlags;
	};
}} // namespace Server::Z21


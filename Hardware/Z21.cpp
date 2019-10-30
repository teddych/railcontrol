/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <cstdint>    //int64_t;
#include <cstdio>     //printf
#include <cstdlib>    //exit(0);
#include <cstring>    //memset
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>   //close;

#include "Hardware/Z21.h"
#include "Utils/Utils.h"

namespace Hardware
{

	// create instance of Z21
	extern "C" Z21* create_Z21(const HardwareParams* params)
	{
		return new Z21(params);
	}

	// delete instance of Z21
	extern "C" void destroy_Z21(Z21* cs2)
	{
		delete(cs2);
	}

	Z21::Z21(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "Z21 / " + params->name + " at IP " + params->arg1),
	 	logger(Logger::Logger::GetLogger("Z21 " + params->name + " " + params->arg1)),
	 	run(true),
	 	senderConnection(logger, params->arg1, Z21Port),
	 	receiverConnection(logger, "0.0.0.0", Z21Port)
	{
		logger->Info(name);
		if (senderConnection.IsConnected())
		{
			logger->Info("Z21 sender socket created");
		}
		else
		{
			logger->Error("Unable to create UDP socket for sending data to Z21");
		}
		receiverThread = std::thread(&Hardware::Z21::Receiver, this);
	}

	Z21::~Z21()
	{
		run = false;
		SendLogOff();
		receiverConnection.Terminate();
		receiverThread.join();
		logger->Info("Terminating Z21 sender");
	}

	void Z21::Booster(const boosterState_t status)
	{
		logger->Info(status ? Languages::TextTurningBoosterOn : Languages::TextTurningBoosterOff);
		unsigned char buffer[7] = { 0x07, 0x00, 0x40, 0x00, 0x21, 0x80, 0xA1 };
		buffer[5] |= status;
		senderConnection.Send(buffer, sizeof(buffer));
	}

	void Z21::LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed)
	{
		logger->Info("Setting speed of cs2 loco {0}/{1} to speed {2}", protocol, address, speed);
	}

	void Z21::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		logger->Info("Setting direction of cs2 loco {0}/{1} to {2}", protocol, address, direction == DirectionRight ? "forward" : "reverse");
	}

	void Z21::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		logger->Info("Setting f{0} of cs2 loco {1}/{2} to \"{3}\"", static_cast<int>(function), static_cast<int>(protocol), static_cast<int>(address), on ? "on" : "off");
	}

	void Z21::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		DataModel::Accessory::Status(state, stateText);
		logger->Info("Setting state of cs2 accessory {0}/{1}/{2} to \"{3}\"", static_cast<int>(protocol), static_cast<int>(address), stateText, on ? "on" : "off");
	}

	// the Receiver thread of the Z21
	void Z21::Receiver()
	{
		Utils::Utils::SetThreadName("Z21");
		logger->Info("Z21 Receiver started");
		if (!receiverConnection.IsConnected())
		{
			logger->Error("Unable to create UDP connection for receiving data from Z21");
			return;
		}

		bool ret = receiverConnection.Bind();
		if (!ret)
		{
			logger->Error("Unable to bind the socket for Z21 Receiver, Closing socket.");
			return;
		}

		SendGetSerialNumber();
		SendGetHardwareInfo();
		SendBroadcastFlags(0x00010001);

		unsigned char buffer[Z21CommandBufferLength];
		while(run)
		{
			ssize_t dataLength = receiverConnection.Receive(buffer, sizeof(buffer));

			if (!run)
			{
				break;
			}

			if (dataLength < 0)
			{
				logger->Error("Unable to receive data from Z21. Closing socket.");
				break;
			}

			if (dataLength == 0)
			{
				continue;
			}

			logger->Debug("Received {0} bytes from Z21", dataLength);
			logger->Hex(buffer, dataLength);

			ssize_t dataRead = 0;
			while (dataRead < dataLength)
			{
				ssize_t ret = InterpretData(buffer, dataLength - dataRead);
				if (ret == -1)
				{
					break;
				}
				dataRead += ret;
			}
		}
		receiverConnection.Terminate();
		logger->Info("Terminating Z21 receiver");
	}

	ssize_t Z21::InterpretData(unsigned char* buffer, size_t bufferLength)
	{
		unsigned short dataLength = Utils::Utils::DataLittleEndianToShort(buffer);
		if (dataLength < 4 || dataLength > bufferLength)
		{
			return -1;
		}

		unsigned short header = Utils::Utils::DataLittleEndianToShort(buffer + 2);
		switch(header)
		{
			case 0x10:
			{
				unsigned int serialNumber = Utils::Utils::DataLittleEndianToInt(buffer + 4);
				logger->Info(Languages::TextSerialNumberIs, serialNumber);
				break;
			}

			case 0x18:
				break;

			case 0x1A:
			{
				unsigned int hardwareType = Utils::Utils::DataLittleEndianToInt(buffer + 4);
				const char* hardwareTypeText;
				switch (hardwareType)
				{
					case 0x00000200:
						hardwareTypeText = Languages::GetText(Languages::TextZ21Black2012);
						break;

					case 0x00000201:
						hardwareTypeText = Languages::GetText(Languages::TextZ21Black2013);
						break;

					case 0x00000202:
						hardwareTypeText = Languages::GetText(Languages::TextZ21SmartRail2012);
						break;

					case 0x00000203:
						hardwareTypeText = Languages::GetText(Languages::TextZ21White2013);
						break;

					case 0x00000204:
						hardwareTypeText = Languages::GetText(Languages::TextZ21Start2016);
						break;

					default:
						hardwareTypeText = Languages::GetText(Languages::TextZ21Unknown);
						break;
				}

				unsigned char firmwareVersionMajor = buffer[9];
				unsigned char firmwareVersionMinor = buffer[8];
				std::string firmwareVersionText = Utils::Utils::IntegerToBCD(firmwareVersionMajor) + "." + Utils::Utils::IntegerToBCD(firmwareVersionMinor);
				logger->Info(Languages::TextZ21Type, hardwareTypeText, firmwareVersionText);
				break;
			}

			case 0x40:
				switch (buffer[8])
				{
					case 0x43:
						break;

					case 0x61:
						switch (buffer[9])
						{
							case 0x00:
								manager->Booster(ControlTypeHardware, BoosterStop);
								break;

							case 0x01:
								manager->Booster(ControlTypeHardware, BoosterGo);
								break;

							case 0x02:
								break;

							case 0x08:
								manager->Booster(ControlTypeHardware, BoosterStop);
								break;

							case 0x12:
								break;

							case 0x13:
								break;

							case 0x82:
								logger->Warning(Languages::TextZ21DoesNotUnderstand);
								break;
						}
						break;

					case 0x62:
						break;

					case 0x63:
						break;

					case 0x64:
						break;

					case 0x81:
						break;

					case 0xEF:
						break;

					case 0xF3:
						break;

					default:
						break;
				}
				break;

			case 0x51:
				break;

			case 0x60:
				break;

			case 0x70:
				break;

			case 0x80:
				break;

			case 0x84:
				break;

			case 0x88:
				break;

			case 0xA0:
				break;

			case 0xA1:
				break;

			case 0xA2:
				break;

			case 0xA3:
				break;

			case 0xA4:
				break;

			case 0xC4:
				break;

			default:
				return -1;
		}
		return dataLength;
	}

	void Z21::SendGetSerialNumber()
	{
		char buffer[4] = { 0x04, 0x00, 0x10, 0x00 };
		senderConnection.Send(buffer, sizeof(buffer));
	}

	void Z21::SendGetHardwareInfo()
	{
		char buffer[4] = { 0x04, 0x00, 0x1A, 0x00 };
		senderConnection.Send(buffer, sizeof(buffer));
	}

	void Z21::SendLogOff()
	{
		char buffer[4] = { 0x04, 0x00, 0x30, 0x00 };
		senderConnection.Send(buffer, sizeof(buffer));
	}

	void Z21::SendBroadcastFlags(const unsigned int flags)
	{
		unsigned char buffer[8] = { 0x08, 0x00, 0x50, 0x00 };
		Utils::Utils::IntToDataLittleEndian(flags, buffer + 4);
		senderConnection.Send(buffer, sizeof(buffer));
	}
} // namespace

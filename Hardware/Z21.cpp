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

#include <cstdint>    //int64_t;
#include <cstdio>     //printf
#include <cstdlib>    //exit(0);
#include <cstring>    //memset
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
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
	extern "C" void destroy_Z21(Z21* z21)
	{
		delete(z21);
	}

	Z21::Z21(const HardwareParams* params)
	:	HardwareInterface(params->GetManager(), params->GetControlID(), "Z21 / " + params->GetName() + " at IP " + params->GetArg1()),
	 	logger(Logger::Logger::GetLogger("Z21 " + params->GetName() + " " + params->GetArg1())),
	 	run(true),
	 	connection(logger, params->GetArg1(), Z21Port)
	{
		logger->Info(Languages::TextStarting, name);

		if (connection.IsConnected())
		{
			logger->Info(Languages::TextSenderSocketCreated);
		}
		else
		{
			logger->Error(Languages::TextUnableToCreateUdpSocket, params->GetArg1(), Z21Port);
		}
		receiverThread = std::thread(&Hardware::Z21::Receiver, this);
		heartBeatThread = std::thread(&Hardware::Z21::HeartBeatSender, this);
		accessorySenderThread = std::thread(&Hardware::Z21::AccessorySender, this);
	}

	Z21::~Z21()
	{
		run = false;
		SendLogOff();
		accessoryQueue.Terminate();
		connection.Terminate();
		accessorySenderThread.join();
		heartBeatThread.join();
		receiverThread.join();
		logger->Info(Languages::TextTerminatingSenderSocket);
	}

	void Z21::Booster(const boosterState_t status)
	{
		logger->Info(status ? Languages::TextTurningBoosterOn : Languages::TextTurningBoosterOff);
		unsigned char buffer[7] = { 0x07, 0x00, 0x40, 0x00, 0x21, 0x80, 0xA1 };
		buffer[5] |= status;
		Send(buffer, sizeof(buffer));
	}

	unsigned char Z21::EncodeSpeed14(const locoSpeed_t speed)
	{
		locoSpeed_t speedInternal = speed >> 6;
		switch (speedInternal)
		{
			case MinSpeed:
				return 0x00;

			case 0x0F:
				return 0x0F;

			default:
				return speedInternal + 1;
		}
	}

	locoSpeed_t Z21::DecodeSpeed14(unsigned char data)
	{
		switch (data)
		{
			case 0x00:
				return MinSpeed;

			case 0x0F:
				return MaxSpeed;

			default:
				return (data - 1) << 6;
		}
	}

	unsigned char Z21::EncodeSpeed28(const locoSpeed_t speed)
	{
		locoSpeed_t speedInternal = speed >> 5;
		switch (speedInternal)
		{
			case MinSpeed:
				return 0x00;

			case 0x1F:
				return 0x1F;

			default:
				++speedInternal;
				return (speedInternal >> 1 | (speedInternal & 0x01) << 4);
		}
	}

	locoSpeed_t Z21::DecodeSpeed28(unsigned char data)
	{
		switch (data)
		{
			case 0x00:
				return MinSpeed;

			case 0x1F:
				return MaxSpeed;

			default:
				unsigned char internalData = ((data >> 4) & 0x01) | ((data & 0x0F) << 1);
				--internalData;
				return internalData << 5;
		}
	}

	unsigned char Z21::EncodeSpeed128(const locoSpeed_t speed)
	{
		locoSpeed_t speedInternal = speed >> 3;
		switch (speedInternal)
		{
			case MinSpeed:
				return 0x00;

			case 0x7F:
				return 0x7F;

			default:
				return speedInternal + 1;
		}
	}

	locoSpeed_t Z21::DecodeSpeed128(unsigned char data)
	{
		switch (data)
		{
			case 0x00:
				return MinSpeed;

			case 0x7F:
				return MaxSpeed;

			default:
				return (data - 1) << 3;
		}
	}

	void Z21::LocoSpeed(const protocol_t protocol, const address_t address, const locoSpeed_t speed)
	{
		direction_t direction = cache.GetDirection(address);
		cache.SetSpeed(address, speed);
		LocoSpeedDirection(protocol, address, speed, direction);
	}

	void Z21::LocoDirection(const protocol_t protocol, const address_t address, const direction_t direction)
	{
		locoSpeed_t speed = cache.GetSpeed(address);
		cache.SetDirection(address, direction);
		LocoSpeedDirection(protocol, address, speed, direction);
	}

	void Z21::LocoSpeedDirection(const protocol_t protocol, const address_t address, const locoSpeed_t speed, const direction_t direction)
	{
		switch (protocol)
		{
			case ProtocolMM1:
			case ProtocolMM15:
			case ProtocolMM2:
				SendSetLocoModeMM(address);
				break;

			case ProtocolDCC14:
			case ProtocolDCC28:
			case ProtocolDCC128:
				SendSetLocoModeDCC(address);
				break;

			default:
				return;
		}

		unsigned char buffer[10] = { 0x0A, 0x00, 0x40, 0x00, 0xE4 };
		switch (protocol)
		{
			case ProtocolMM1:
			case ProtocolDCC14:
				buffer[5] = 0x10;
				buffer[8] = EncodeSpeed14(speed);
				break;

			case ProtocolMM15:
			case ProtocolDCC28:
				buffer[5] = 0x12;
				buffer[8] = EncodeSpeed28(speed);
				break;

			case ProtocolMM2:
			case ProtocolDCC128:
				buffer[5] = 0x13;
				buffer[8] = EncodeSpeed128(speed);
				break;

			default:
				return;
		}
		Utils::Utils::ShortToDataBigEndian(address | 0xC000, buffer + 6);
		buffer[8] |=  static_cast<unsigned char>(direction) << 7;
		buffer[9] = buffer[4] ^ buffer[5] ^ buffer[6] ^ buffer[7] ^ buffer[8];
		Send(buffer, sizeof(buffer));
	}

	void Z21::LocoFunction(__attribute__ ((unused)) const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		unsigned char buffer[10] = { 0x0A, 0x00, 0x40, 0x00, 0xE4, 0xF8 };
		Utils::Utils::ShortToDataBigEndian(address | 0xC000, buffer + 6);
		buffer[8] = (static_cast<unsigned char>(on) << 6) | (function & 0x3F);
		buffer[9] = buffer[4] ^ buffer[5] ^ buffer[6] ^ buffer[7] ^ buffer[8];
		Send(buffer, sizeof(buffer));
	}

	void Z21::LocoSpeedDirectionFunctions(const protocol_t protocol, const address_t address, const locoSpeed_t speed, const direction_t direction, std::vector<bool>& functions)
	{
		cache.SetSpeedDirection(address, speed, direction);
		LocoSpeedDirection(protocol, address, speed, direction);
		for (size_t functionNr = 0; functionNr < functions.size(); ++functionNr)
		{
			LocoFunction(protocol, address, functionNr, functions[functionNr]);
		}
	}

	void Z21::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const waitTime_t waitTime)
	{
		AccessoryQueueEntry entry(protocol, address, state, waitTime);
		accessoryQueue.Enqueue(entry);
	}

	void Z21::AccessoryOn(const protocol_t protocol, const address_t address, const accessoryState_t state)
	{
		switch (protocol)
		{
			case ProtocolMM:
				SendSetTurnoutModeMM(address);
				break;

			case ProtocolDCC:
				SendSetTurnoutModeDCC(address);
				break;

			default:
				return;
		}
		AccessoryOnOrOff(protocol, address, state, true);
	}

	void Z21::AccessoryOff(const protocol_t protocol, const address_t address, const accessoryState_t state)
	{
		AccessoryOnOrOff(protocol, address, state, false);
	}

	void Z21::AccessoryOnOrOff(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		const address_t zeroBasedAddress = address - 1;
		switch (protocol)
		{
			case ProtocolMM:
				SendSetTurnoutModeMM(zeroBasedAddress);
				break;

			case ProtocolDCC:
				SendSetTurnoutModeDCC(zeroBasedAddress);
				break;

			default:
				return;
		}

		unsigned char buffer[9] = { 0x09, 0x00, 0x40, 0x00, 0x53 };
		Utils::Utils::ShortToDataBigEndian(zeroBasedAddress, buffer + 5);
		buffer[7] = 0x80 | (static_cast<unsigned char>(on) << 3) | static_cast<unsigned char>(state);
		buffer[8] = buffer[4] ^ buffer[5] ^ buffer[6] ^ buffer[7];
		Send(buffer, sizeof(buffer));
	}

	void Z21::AccessorySender()
	{
		Utils::Utils::SetThreadName("Z21 Accessory Sender");
		logger->Info(Languages::TextAccessorySenderThreadStarted);
		while (run)
		{
			AccessoryQueueEntry entry = accessoryQueue.Dequeue();
			if (entry.protocol == ProtocolNone)
			{
				continue;
			}
			AccessoryOn(entry.protocol, entry.address, entry.state);
			std::this_thread::sleep_for(std::chrono::milliseconds(entry.waitTime));
			AccessoryOff(entry.protocol, entry.address, entry.state);
		}
		logger->Info(Languages::TextTerminatingAccessorySenderThread);
	}

	void Z21::HeartBeatSender()
	{
		Utils::Utils::SetMinThreadPriority();
		Utils::Utils::SetThreadName("Z21 Heartbeat Sender");
		logger->Info(Languages::TextHeartBeatThreadStarted);
		unsigned int counter = 0;
		while(run)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			++counter;
			counter &= 0x0F;
			if (counter != 0)
			{
				continue;
			}
			SendGetStatus();
		}
		logger->Info(Languages::TextTerminatingHeartBeatThread);
	}

	void Z21::Receiver()
	{
		Utils::Utils::SetThreadName("Z21 Receiver");
		logger->Info(Languages::TextReceiverThreadStarted);

		SendGetSerialNumber();
		SendGetHardwareInfo();
		SendBroadcastFlags(static_cast<broadCastFlags_t>(BroadCastFlagBasic
			| BroadCastFlagRBus
			| BroadCastFlagSystemState
			| BroadCastFlagAllLoco
			| BroadCastFlagCanDetector
			| BroadCastFlagLocoNetBasic
			| BroadCastFlagLocoNetLoco
			| BroadCastFlagLocoNetSwitch
			| BroadCastFlagLocoNetDetector));

		unsigned char buffer[Z21CommandBufferLength];
		while(run)
		{
			ssize_t dataLength = connection.Receive(buffer, sizeof(buffer));

			if (run == false)
			{
				break;
			}

			if (dataLength < 0)
			{
				logger->Error(Languages::TextUnableToReceiveData);
				break;
			}

			if (dataLength == 0)
			{
				continue;
			}

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
		logger->Info(Languages::TextTerminatingReceiverThread);
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
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
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
				switch (buffer[4])
				{
					case 0x43:
						logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
						break;

					case 0x61:
						switch (buffer[5])
						{
							case 0x00:
								manager->Booster(ControlTypeHardware, BoosterStop);
								break;

							case 0x01:
								manager->Booster(ControlTypeHardware, BoosterGo);
								break;

							case 0x02:
								logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
								break;

							case 0x08:
								manager->Booster(ControlTypeHardware, BoosterStop);
								break;

							case 0x12:
								logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
								break;

							case 0x13:
								logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
								break;

							case 0x82:
								logger->Warning(Languages::TextZ21DoesNotUnderstand);
								break;
						}
						break;

					case 0x62:
						logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
						break;

					case 0x63:
						logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
						break;

					case 0x64:
						logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
						break;

					case 0x81:
						logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
						break;

					case 0xEF:
					{
						// FIXME: MM is not recognized
						address_t address = Utils::Utils::DataBigEndianToInt(buffer + 5) | 0x3FFF;
						bool used = (buffer[6] >> 3) & 0x01;
						logger->Debug(used ? "Fremd gesteuert" : "RailControl gesteuert");
						unsigned char protocolType = buffer[6] & 0x07;
						protocol_t protocol;
						unsigned char speedData = buffer[7] & 0x7F;
						locoSpeed_t newSpeed;
						switch (protocolType)
						{
							case 0:
								protocol = ProtocolDCC14;
								newSpeed = DecodeSpeed14(speedData);
								break;

							case 2:
								protocol = ProtocolDCC28;
								newSpeed = DecodeSpeed28(speedData);
								break;

							case 4:
								protocol = ProtocolDCC128;
								newSpeed = DecodeSpeed128(speedData);
								break;

							default:
								return dataLength;
						}
						locoSpeed_t oldSpeed = cache.GetSpeed(address);
						if (newSpeed != oldSpeed)
						{
							cache.SetSpeed(address, newSpeed);
							manager->LocoSpeed(ControlTypeHardware, controlID, protocol, address, newSpeed);
						}
						direction_t newDirection = (buffer[7] >> 7) ? DirectionRight : DirectionLeft;
						direction_t oldDirection = cache.GetDirection(address);
						if (newDirection != oldDirection)
						{
							cache.SetDirection(address, newDirection);
							manager->LocoDirection(ControlTypeHardware, controlID, protocol, address, newDirection);
						}
						break;
					}

					case 0xF3:
						logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
						break;

					default:
						break;
				}
				break;

			case 0x51:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0x60:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0x70:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0x80:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0x84:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0x88:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0xA0:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0xA1:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0xA2:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0xA3:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0xA4:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			case 0xC4:
				logger->Warning(Languages::TextNotImplemented, __FILE__, __LINE__);
				break;

			default:
				logger->Warning(Languages::TextZ21DoesNotUnderstand);
				return -1;
		}
		return dataLength;
	}

	void Z21::SendGetSerialNumber()
	{
		char buffer[4] = { 0x04, 0x00, 0x10, 0x00 };
		Send(buffer, sizeof(buffer));
	}

	void Z21::SendGetHardwareInfo()
	{
		char buffer[4] = { 0x04, 0x00, 0x1A, 0x00 };
		Send(buffer, sizeof(buffer));
	}

	void Z21::SendGetStatus()
	{
		char buffer[7] = { 0x07, 0x00, 0x40, 0x00, 0x21, 0x24, 0x05 };
		Send(buffer, sizeof(buffer));
	}

	void Z21::SendLogOff()
	{
		char buffer[4] = { 0x04, 0x00, 0x30, 0x00 };
		Send(buffer, sizeof(buffer));
	}

	void Z21::SendBroadcastFlags(const broadCastFlags_t flags)
	{
		unsigned char buffer[8] = { 0x08, 0x00, 0x50, 0x00 };
		Utils::Utils::IntToDataLittleEndian(flags, buffer + 4);
		Send(buffer, sizeof(buffer));
	}

	void Z21::SendSetMode(const address_t address, const commands_t command, const protocolMode_t mode)
	{
		switch (command)
		{
			case CommandSetLocoMode:
			case CommandSetTurnoutMode:
				break;

			default:
				return;
		}

		switch (mode)
		{
			case ProtocolModeMM:
				if (address > MaxMMAddress)
				{
					return;
				}
				break;

			case ProtocolModeDCC:
				break;

			default:
				return;
		}

		unsigned char buffer[7] = { 0x07, 0x00, command, 0x00, 0x00, 0x00, mode };
		Utils::Utils::ShortToDataBigEndian(address, buffer + 4);
		Send(buffer, sizeof(buffer));
	}

	void Z21::SendSetLocoModeMM(const address_t address)
	{
		SendSetMode(address, CommandSetLocoMode, ProtocolModeMM);
	}

	void Z21::SendSetLocoModeDCC(const address_t address)
	{
		SendSetMode(address, CommandSetLocoMode, ProtocolModeDCC);
	}

	void Z21::SendSetTurnoutModeMM(const address_t address)
	{
		const address_t zeroBasedAddress = address - 1;
		SendSetMode(zeroBasedAddress, CommandSetTurnoutMode, ProtocolModeMM);
	}

	void Z21::SendSetTurnoutModeDCC(const address_t address)
	{
		const address_t zeroBasedAddress = address - 1;
		SendSetMode(zeroBasedAddress, CommandSetTurnoutMode, ProtocolModeDCC);
	}

	int Z21::Send(const unsigned char* buffer, const size_t bufferLength)
	{
		logger->Hex(buffer, bufferLength);
		return connection.Send(buffer, bufferLength);
	}
} // namespace

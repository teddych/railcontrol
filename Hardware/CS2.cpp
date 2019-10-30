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

#include "Hardware/CS2.h"
#include "Utils/Utils.h"

namespace Hardware
{

	// create instance of cs2
	extern "C" CS2* create_CS2(const HardwareParams* params)
	{
		return new CS2(params);
	}

	// delete instance of cs2
	extern "C" void destroy_CS2(CS2* cs2)
	{
		delete(cs2);
	}

	CS2::CS2(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "Maerklin Central Station 2 (CS2) / " + params->name + " at IP " + params->arg1),
	 	logger(Logger::Logger::GetLogger("CS2 " + params->name + " " + params->arg1)),
	 	run(true),
	 	senderConnection(logger, params->arg1, CS2SenderPort),
	 	receiverConnection(logger, "0.0.0.0", CS2ReceiverPort)
	{
		logger->Info(name);
		if (senderConnection.IsConnected())
		{
			logger->Info("CS2 sender socket created");
		}
		else
		{
			logger->Error("Unable to create UDP socket for sending data to CS2");
		}
		receiverThread = std::thread(&Hardware::CS2::Receiver, this);
	}

	CS2::~CS2()
	{
		run = false;
		receiverConnection.Terminate();
		receiverThread.join();
		logger->Info("Terminating CS2 sender");
	}

	void CS2::CreateCommandHeader(unsigned char* buffer, const cs2Prio_t prio, const cs2Command_t command, const cs2Response_t response, const cs2Length_t length)
	{
		buffer[0] = (prio << 1) | (command >> 7);
		buffer[1] = (command << 1) | (response & 0x01);
		buffer[2] = (hash >> 8);
		buffer[3] = (hash & 0xFF);
		buffer[4] = length;
	}

	void CS2::ReadCommandHeader(unsigned char* buffer, cs2Prio_t& prio, cs2Command_t& command, cs2Response_t& response, cs2Length_t& length, cs2Address_t& address, protocol_t& protocol)
	{
		prio = buffer[0] >> 1;
		command = (cs2Command_t)(buffer[0]) << 7 | (cs2Command_t)(buffer[1]) >> 1;
		response = buffer[1] & 0x01;
		length = buffer[4];
		address = Utils::Utils::DataBigEndianToInt(buffer + 5);
		cs2Address_t maskedAddress = address & 0x0000FC00;
		address &= 0x03FF;

		switch (maskedAddress)
		{
			case 0x3800:
			case 0x3C00:
			case 0xC000:
				protocol = ProtocolDCC;
				return;

			case 0x4000:
				protocol = ProtocolMFX;
				return;

			default:
				protocol = ProtocolMM2;
				return;
		}
	}

	void CS2::CreateLocID(unsigned char* buffer, const protocol_t& protocol, const address_t& address)
	{
		uint32_t locID = address;
		if (protocol == ProtocolDCC)
		{
			locID |= 0xC000;
		}
		else if (protocol == ProtocolMFX)
		{
			locID |= 0x4000;
		}
		// else expect PROTOCOL_MM2: do nothing
		Utils::Utils::IntToDataBigEndian(locID, buffer);
	}

	void CS2::CreateAccessoryID(unsigned char* buffer, const protocol_t& protocol, const address_t& address)
	{
		uint32_t locID = address;
		if (protocol == ProtocolDCC)
		{
			locID |= 0x3800;
		}
		else
		{
			locID |= 0x3000;
		}
		Utils::Utils::IntToDataBigEndian(locID, buffer);
	}

	// turn booster on or off
	void CS2::Booster(const boosterState_t status)
	{
		logger->Info("Turning CS2 booster {0}", status ? "on" : "off");
		unsigned char buffer[CS2CommandBufferLength];
		// fill up header & locid
		CreateCommandHeader(buffer, 0, 0x00, 0, 5);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		//buffer[5-8]: 0 = all
		//buffer[9]: subcommand stop 0x01
		buffer[9] = status;

		// send data
		if (senderConnection.Send(buffer, sizeof(buffer)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// set the speed of a loco
	void CS2::LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed)
	{
		logger->Info("Setting speed of cs2 loco {0}/{1} to speed {2}", protocol, address, speed);
		unsigned char buffer[CS2CommandBufferLength];
		// set header
		CreateCommandHeader(buffer, 0, 0x04, 0, 6);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		CreateLocID(buffer + 5, protocol, address);
		// set speed
		buffer[9] = (speed >> 8);
		buffer[10] = (speed & 0xFF);

		// send data
		if (senderConnection.Send(buffer, sizeof(buffer)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// set the direction of a loco
	void CS2::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		logger->Info("Setting direction of cs2 loco {0}/{1} to {2}", protocol, address, direction == DirectionRight ? "forward" : "reverse");
		unsigned char buffer[CS2CommandBufferLength];
		// set header
		CreateCommandHeader(buffer, 0, 0x05, 0, 5);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		CreateLocID(buffer + 5, protocol, address);
		// set speed
		buffer[9] = (direction ? 1 : 2);

		// send data
		if (senderConnection.Send(buffer, sizeof(buffer)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// set loco function
	void CS2::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		logger->Info("Setting f{0} of cs2 loco {1}/{2} to \"{3}\"", static_cast<int>(function), static_cast<int>(protocol), static_cast<int>(address), on ? "on" : "off");
		unsigned char buffer[CS2CommandBufferLength];
		// set header
		CreateCommandHeader(buffer, 0, 0x06, 0, 6);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		CreateLocID(buffer + 5, protocol, address);
		buffer[9] = function;
		buffer[10] = on;

		// send data
		if (senderConnection.Send(buffer, sizeof(buffer)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	void CS2::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		DataModel::Accessory::Status(state, stateText);
		logger->Info("Setting state of cs2 accessory {0}/{1}/{2} to \"{3}\"", static_cast<int>(protocol), static_cast<int>(address), stateText, on ? "on" : "off");
		unsigned char buffer[CS2CommandBufferLength];
		// set header
		CreateCommandHeader(buffer, 0, 0x0B, 0, 6);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		CreateAccessoryID(buffer + 5, protocol, address - 1); // GUI-address is 1-based, protocol-address is 0-based
		buffer[9] = state & 0x03;
		buffer[10] = static_cast<unsigned char>(on);

		// send data
		if (senderConnection.Send(buffer, sizeof(buffer)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// the receiver thread of the CS2
	void CS2::Receiver()
	{
		Utils::Utils::SetThreadName("CS2");
		logger->Info("CS2 receiver started");
		if (!receiverConnection.IsConnected())
		{
			logger->Error("Unable to create UDP connection for receiving data from CS2");
			return;
		}

		bool ret = receiverConnection.Bind();
		if (!ret)
		{
			logger->Error("Unable to bind the socket for CS2 receiver, Closing socket.");
			return;
		}
		unsigned char buffer[CS2CommandBufferLength];
		while(run)
		{
			ssize_t datalen = receiverConnection.Receive(buffer, sizeof(buffer));

			if (!run)
			{
				break;
			}

			if (datalen < 0)
			{
				logger->Error("Unable to receive data from CS2. Closing socket.");
				break;
			}

			if (datalen != 13)
			{
				logger->Error("Unable to receive valid data from CS2. Continuing with next packet.");
				continue;
			}

			//xlog("Receiver %i bytes received", datalen);
			//hexlog(buffer, datalen);
			cs2Prio_t prio;
			cs2Command_t command;
			cs2Response_t response;
			cs2Length_t length;
			cs2Address_t address;
			protocol_t protocol;
			ReadCommandHeader(buffer, prio, command, response, length, address, protocol);
			if (command == 0x11 && response)
			{
				// s88 event
				std::string text;
				DataModel::Feedback::feedbackState_t state;
				if (buffer[10])
				{
					text = "on";
					state = DataModel::Feedback::FeedbackStateOccupied;
				}
				else
				{
					text = "off";
					state = DataModel::Feedback::FeedbackStateFree;
				}
				logger->Info("S88 Pin {0} set to {1}", address, text);
				manager->FeedbackState(controlID, address, state);
			}
			else if (command == 0x00 && !response && length == 5)
			{
				unsigned char subcmd = buffer[9];
				switch (subcmd)
				{
					case 0x00:
						// system stopp
						manager->Booster(ControlTypeHardware, BoosterStop);
						break;

					case 0x01:
						// system go
						manager->Booster(ControlTypeHardware, BoosterGo);
						break;
				}
			}
			else if (command == 0x04 && !response && length == 6)
			{
				// speed event
				locoSpeed_t speed = Utils::Utils::DataBigEndianToShort(buffer + 9);
				logger->Info("Received loco speed command for protocol {0} address {1} and speed {2}", protocol, address, speed);
				manager->LocoSpeed(ControlTypeHardware, controlID, protocol, static_cast<address_t>(address), speed);
			}
			else if (command == 0x05 && !response && length == 5)
			{
				// direction event (implies speed=0)
				direction_t direction = (buffer[9] == 1 ? DirectionRight : DirectionLeft);
				logger->Info("Received loco direction command for protocol {0} address {1} and direction {2}", protocol, address, direction);
				manager->LocoSpeed(ControlTypeHardware, controlID, protocol, static_cast<address_t>(address), MinSpeed);
				manager->LocoDirection(ControlTypeHardware, controlID, protocol, static_cast<address_t>(address), direction);
			}
			else if (command == 0x06 && !response && length == 6)
			{
				// function event
				function_t function = buffer[9];
				bool on = buffer[10] != 0;
				logger->Info("Received loco function command for protocol {0} address {1} and function {2} state {3}", protocol, address, function, on);
				manager->LocoFunction(ControlTypeHardware, controlID, protocol, static_cast<address_t>(address), function, on);
			}
			else if (command == 0x0B && !response && length == 6 && buffer[10] == 1)
			{
				// accessory event
				accessoryState_t state = buffer[9];
				// GUI-address is 1-based, protocol-address is 0-based
				++address;
				logger->Info("Received accessory command for protocol {0} address {1} and state {2}", protocol, address, state);
				manager->AccessoryState(ControlTypeHardware, controlID, protocol, address, state);
			}
		}
		receiverConnection.Terminate();
		logger->Info("Terminating CS2 receiver");
	}
} // namespace

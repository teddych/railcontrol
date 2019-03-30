#include <cstdint>    //int64_t;
#include <cstdio>     //printf
#include <cstdlib>    //exit(0);
#include <cstring>    //memset
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>   //close;

#include "text/converters.h"
#include "hardware/cs2.h"
#include "util.h"

#define CS2_CMD_BUF_LEN 13    // Length of the commandbuffer
#define CS2_PORT_SEND 15731   // The port on which to send data
#define CS2_PORT_RECV 15730   // The port on which to receive data

namespace hardware
{

	// create instance of cs2
	extern "C" CS2* create_cs2(const HardwareParams* params)
	{
		return new CS2(params);
	}

	// delete instance of cs2
	extern "C" void destroy_cs2(CS2* cs2)
	{
		delete(cs2);
	}

	// start the thing
	CS2::CS2(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "Maerklin Central Station 2 (CS2) / " + params->name + " at IP " + params->arg1),
	 	logger(Logger::Logger::GetLogger("CS2 " + params->name + " " + params->arg1))
	{
		logger->Info(name);
		run = true;
		senderSocket = CreateUdpConnection((struct sockaddr*)&sockaddr_inSender, sizeof(struct sockaddr_in), params->arg1.c_str(), CS2_PORT_SEND);
		if (senderSocket < 0)
		{
			logger->Error("Unable to create UDP socket for sending data to CS2");
		}
		else
		{
			logger->Info("CS2 sender socket created");
		}
		receiverThread = std::thread([this] {receiver();});
	}

	// stop the thing
	CS2::~CS2()
	{
		run = false;
		close (senderSocket);
		logger->Info("CS2 sender socket closed");
		receiverThread.join();
	}

	void CS2::createCommandHeader(char* buffer, const cs2Prio_t prio, const cs2Command_t command, const cs2Response_t response, const cs2Length_t length)
	{
		buffer[0] = (prio << 1) | (command >> 7);
		buffer[1] = (command << 1) | (response & 0x01);
		buffer[2] = (hash >> 8);
		buffer[3] = (hash & 0xFF);
		buffer[4] = length;
	}

	void CS2::intToData(const uint32_t i, char* buffer)
	{
		buffer[0] = (i >> 24);
		buffer[1] = ((i >> 16) & 0xFF);
		buffer[2] = ((i >> 8) & 0xFF);
		buffer[3] = (i & 0xFF);
	}

	uint32_t CS2::dataToInt(const char* buffer)
	{
		uint32_t i = (const unsigned char)buffer[0];
		i <<= 8;
		i |= (const unsigned char)buffer[1];
		i <<= 8;
		i |= (const unsigned char)buffer[2];
		i <<= 8;
		i |= (const unsigned char)buffer[3];
		return i;
	}

	uint16_t CS2::dataToShort(const char* buffer)
	{
		uint16_t i = (const unsigned char)buffer[0];
		i <<= 8;
		i |= (const unsigned char)buffer[1];
		return i;
	}

	void CS2::readCommandHeader(char* buffer, cs2Prio_t& prio, cs2Command_t& command, cs2Response_t& response, cs2Length_t& length, cs2Address_t& address, protocol_t& protocol)
	{
		prio = buffer[0] >> 1;
		command = (cs2Command_t)(buffer[0]) << 7 | (cs2Command_t)(buffer[1]) >> 1;
		response = buffer[1] & 0x01;
		length = buffer[4];
		address = dataToInt(buffer + 5);
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

	void CS2::createLocID(char* buffer, const protocol_t& protocol, const address_t& address)
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
		intToData(locID, buffer);
	}

	void CS2::createAccessoryID(char* buffer, const protocol_t& protocol, const address_t& address)
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
		intToData(locID, buffer);
	}

	// turn booster on or off
	void CS2::Booster(const boosterState_t status)
	{
		logger->Info("Turning CS2 booster {0}", status ? "on" : "off");
		char buffer[CS2_CMD_BUF_LEN];
		// fill up header & locid
		createCommandHeader(buffer, 0, 0x00, 0, 5);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		//buffer[5-8]: 0 = all
		//buffer[9]: subcommand stop 0x01
		buffer[9] = status;

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// set the speed of a loco
	void CS2::LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed)
	{
		logger->Info("Setting speed of cs2 loco {0}/{1} to speed {2}", protocol, address, speed);
		char buffer[CS2_CMD_BUF_LEN];
		// set header
		createCommandHeader(buffer, 0, 0x04, 0, 6);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		createLocID(buffer + 5, protocol, address);
		// set speed
		buffer[9] = (speed >> 8);
		buffer[10] = (speed & 0xFF);

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// set the direction of a loco
	void CS2::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		logger->Info("Setting direction of cs2 loco {0}/{1} to {2}", protocol, address, direction == DirectionRight ? "forward" : "reverse");
		char buffer[CS2_CMD_BUF_LEN];
		// set header
		createCommandHeader(buffer, 0, 0x05, 0, 5);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		createLocID(buffer + 5, protocol, address);
		// set speed
		buffer[9] = (direction ? 1 : 2);

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// set loco function
	void CS2::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		logger->Info("Setting f{0} of cs2 loco {1}/{2} to \"{3}\"", static_cast<int>(function), static_cast<int>(protocol), static_cast<int>(address), on ? "on" : "off");
		char buffer[CS2_CMD_BUF_LEN];
		// set header
		createCommandHeader(buffer, 0, 0x06, 0, 6);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		createLocID(buffer + 5, protocol, address);
		buffer[9] = function;
		buffer[10] = on;

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	void CS2::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		text::Converters::accessoryStatus(state, stateText);
		logger->Info("Setting state of cs2 accessory {0}/{1}/{2} to \"{3}\"", static_cast<int>(protocol), static_cast<int>(address), stateText, on ? "on" : "off");
		char buffer[CS2_CMD_BUF_LEN];
		// set header
		createCommandHeader(buffer, 0, 0x0B, 0, 6);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		createAccessoryID(buffer + 5, protocol, address - 1); // GUI-address is 1-based, protocol-address is 0-based
		buffer[9] = state & 0x03;
		buffer[10] = static_cast<unsigned char>(on);

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			logger->Error("Unable to send data to CS2");
		}
	}

	// the receiver thread of the CS2
	void CS2::receiver()
	{
		logger->Info("CS2 receiver started");
		struct sockaddr_in sockaddr_in;
		int sock;
		sock = CreateUdpConnection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), "0.0.0.0", CS2_PORT_RECV);
		if (sock < 0)
		{
			logger->Error("Unable to create UDP connection for receiving data from CS2");
			return;
		}

		if (bind(sock, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1)
		{
			logger->Error("Unable to bind the socket for CS2 receiver, Closing socket.");
			close(sock);
			return;
		}
		char buffer[CS2_CMD_BUF_LEN];
		while(run)
		{
			ssize_t datalen;
			do
			{
				//try to receive some data, this is a blocking call
				errno = 0;
				datalen = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
			} while (datalen < 0 && errno == EAGAIN && run);

			if (datalen < 0 && errno != EAGAIN)
			{
				logger->Error("Unable to receive data from CS2. Closing socket.");
				close(sock);
				return;
			}

			if (datalen != 13)
			{
				if (run)
				{
					logger->Error("Unable to receive valid data from CS2. Continuing with next packet.");
					break;
				}
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
			readCommandHeader(buffer, prio, command, response, length, address, protocol);
			if (command == 0x11 && response)
			{
				// s88 event
				std::string text;
				feedbackState_t state;
				if (buffer[10])
				{
					text = "on";
					state = FeedbackStateOccupied;
				}
				else
				{
					text = "off";
					state = FeedbackStateFree;
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
				locoSpeed_t speed = dataToShort(buffer + 9);
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
		close(sock);
		logger->Info("CS2 receiver ended");
	}

	int CS2::CreateUdpConnection(const struct sockaddr* sockaddr, const unsigned int sockaddr_len, const char* server, const unsigned short port) {
		int sock;

		// create socket
		if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		{
			return -1;
		}

		// setting listening port
		memset ((char*)sockaddr, 0, sockaddr_len);
		struct sockaddr_in* sockaddr_in = (struct sockaddr_in*)sockaddr;
		sockaddr_in->sin_family = AF_INET;
		sockaddr_in->sin_port = htons(port);
		if (inet_aton (server, &sockaddr_in->sin_addr) == 0)
		{
			return -2;
		}

		// setting receive timeout to 1s
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
		return sock;
	}
} // namespace

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
	CS2::CS2(const HardwareParams* params) :
		manager(params->manager)
	{
		std::stringstream ss;
		ss << "Maerklin Central Station 2 (CS2) / " << params->name << " at IP " << params->ip;
		name = ss.str();
		xlog(name.c_str());
		run = true;
		senderSocket = create_udp_connection((struct sockaddr*)&sockaddr_inSender, sizeof(struct sockaddr_in), params->ip.c_str(), CS2_PORT_SEND);
		if (senderSocket < 0)
		{
			xlog("Unable to create UDP socket for sending data to CS2");
		}
		else
		{
			xlog("CS2 sender socket created");
		}
		receiverThread = std::thread([this] {receiver();});
	}

	// stop the thing
	CS2::~CS2()
	{
		run = false;
		close (senderSocket);
		xlog("CS2 sender socket closed");
		receiverThread.join();
	}

	void CS2::GetProtocols(std::vector<protocol_t>& protocols) const
	{
		protocols.push_back(ProtocolMM2);
		protocols.push_back(ProtocolMFX);
		protocols.push_back(ProtocolDCC);
	}

	bool CS2::ProtocolSupported(protocol_t protocol) const
	{
		return (protocol == ProtocolMM2 || protocol == ProtocolMFX || protocol == ProtocolDCC);
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
	void CS2::Booster(const boosterStatus_t status)
	{
		if (status)
		{
			xlog("Turning CS2 booster on");
		}
		else
		{
			xlog("Turning CS2 booster off");
		}
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
			xlog("Unable to send data to CS2");
		}
	}

	// set the speed of a loco
	void CS2::SetLocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed)
	{
		xlog("Setting speed of cs2 loco %i/%i to speed %i", protocol, address, speed);
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
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1) {
			xlog("Unable to send data to CS2");
		}
	}

	// set the direction of a loco
	void CS2::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		xlog("Setting direction of cs2 loco %i/%i to %s", protocol, address, direction ? "forward" : "reverse");
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
			xlog("Unable to send data to CS2");
		}
	}

	// set loco function
	void CS2::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		xlog("Setting f%i of cs2 loco %i/%i to \"%s\"", (int)function, (int)protocol, (int)address, on ? "on" : "off");
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

		hexlog(buffer, sizeof(buffer));

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			xlog("Unable to send data to CS2");
		}
	}

	void CS2::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		text::Converters::accessoryStatus(state, stateText);
		xlog("Setting state of cs2 accessory %i/%i/%s to \"%s\"", (int)protocol, (int)address, stateText.c_str(), on ? "on" : "off");
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

		hexlog(buffer, sizeof(buffer));

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1)
		{
			xlog("Unable to send data to CS2");
		}
	}

	// the receiver thread of the CS2
	void CS2::receiver()
	{
		xlog("CS2 receiver started");
		struct sockaddr_in sockaddr_in;
		int sock;
		sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), "0.0.0.0", CS2_PORT_RECV);
		if (sock < 0)
		{
			xlog("Unable to create UDP connection for receiving data from CS2");
			return;
		}

		if (bind(sock, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1)
		{
			xlog("Unable to bind the socket for CS2 receiver");
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
				xlog("Unable to receive data from CS2. Closing socket.");
				close(sock);
				return;
			}
			else if (datalen == 13)
			{
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
					const char* text;
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
					xlog("CS2 S88 Pin %u set to %s", address, text);
					manager->feedback(ControlTypeHardware, address, state);
				}
				else if (command == 0x04 && !response && length == 6)
				{
					// speed event
					LocoSpeed speed = dataToShort(buffer + 9);
					manager->locoSpeed(ControlTypeHardware, protocol, static_cast<address_t>(address), speed);
				}
				else if (command == 0x05 && !response && length == 5)
				{
					// direction event (implies speed=0)
					direction_t direction = (buffer[9] == 1 ? DirectionRight : DirectionLeft);
					manager->locoSpeed(ControlTypeHardware, protocol, static_cast<address_t>(address), 0);
					manager->locoDirection(ControlTypeHardware, protocol, static_cast<address_t>(address), direction);
				}
				else if (command == 0x0B && !response && length == 6 && buffer[10] == 1)
				{
					// accessory event
					accessoryState_t state = buffer[9];
					// GUI-address is 1-based, protocol-address is 0-based
					++address;
					manager->accessory(ControlTypeHardware, protocol, address, state);
				}
			}
			else if (run)
			{
				xlog("Unable to receive valid data from CS2. Continuing with next packet.");
			}
		}
		close(sock);
		xlog("CS2 receiver ended");
	}

} // namespace

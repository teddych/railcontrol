#include <arpa/inet.h>
#include <cstdint>    //int64_t;
#include <cstdio>     //printf
#include <cstdlib>    //exit(0);
#include <cstring>    //memset
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>   //close;

#include "../util.h"
#include "cs2.h"

#define CS2_IP "192.168.0.190"
#define CS2_CMD_BUF_LEN 13    // Length of the commandbuffer
//#define CS2_DATA_BUF_LEN 8    // Length of the databuffer
#define CS2_PORT_SEND 15731   // The port on which to send data
#define CS2_PORT_RECV 15730   // The port on which to receive data

namespace hardware {

	// create instance of cs2
	extern "C" CS2* create_cs2(const HardwareParams* params) {
		return new CS2(params);
  }

	// delete instance of cs2
  extern "C" void destroy_cs2(CS2* cs2) {
    delete(cs2);
  }

  // start the thing
	CS2::CS2(const HardwareParams* params) {
		std::stringstream ss;
		ss << "Maerklin Central Station 2 (CS2) / " << params->name;
		name = ss.str();
		run = true;
    senderSocket = create_udp_connection((struct sockaddr*)&sockaddr_inSender, sizeof(struct sockaddr_in), CS2_IP, CS2_PORT_SEND);
    if (senderSocket < 0) {
      xlog("Unable to create UDP socket for sending data to CS2");
    }
		else {
			xlog("CS2 sender socket created");
		}
		receiverThread = std::thread([this] {receiver();});
	}

  // stop the thing
	CS2::~CS2() {
		run = false;
    close (senderSocket);
    xlog("CS2 sender socket closed");
		receiverThread.join();
	}

  // return the name
	std::string CS2::getName() const {
		return name;
  }

	void CS2::createCommandHeader(char* buffer, const cs2Prio_t& prio, const cs2Command_t& command, const cs2Response_t& response, const cs2Length_t& length) {
		buffer[0] = (prio << 1) | (command >> 7);
		buffer[1] = (command << 1) | (response & 0x01);
		buffer[2] = (hash >> 8);
		buffer[3] = (hash & 0xFF);
		buffer[4] = length;
	}

	void CS2::createLocID(char* buffer, const protocol_t& protocol, const address_t& address) {
		uint32_t locID = address;
		if (protocol == PROTOCOL_DCC) {
			locID |= 0xC000;
		}
		// else expect PROTOCOL_MM2: do nothing
		buffer[0] = (locID >> 24);
		buffer[1] = ((locID >> 16) & 0xFF);
		buffer[2] = ((locID >> 8) & 0xFF);
		buffer[3] = (locID & 0xFF);
	}

	// GO-command (turn on booster)
  void CS2::go() {
		char buffer[CS2_CMD_BUF_LEN];
		// fill up header & locid
		createCommandHeader(buffer, 0, 0x00, 0, 5);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		//buffer[5-8]: 0 = all
		//buffer[9]: subcommand stop 0x01
		buffer[9] = 0x01;

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1) {
			xlog("Unable to send data to CS2");
			return;
		}
		xlog("Starting CS2");
  }

	// Stop-command (turn off booster)
  void CS2::stop() {
		char buffer[CS2_CMD_BUF_LEN];
		// fill up header & locid
		createCommandHeader(buffer, 0, 0x00, 0, 5);
		// set data buffer (8 bytes) to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		//buffer[5-8]: 0 = all
		//buffer[9]: subcommand stop 0x00 (is alreay 0x00)

		// send data
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1) {
			xlog("Unable to send data to CS2");
			return;
		}
		xlog("Stopping CS2");
  }

	// set the speed of a loco
	void CS2::locoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) {
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

	// set loco function
	void CS2::locoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) {
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
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in)) == -1) {
			xlog("Unable to send data to CS2");
		}
	}

  // the receiver thread of the CS2
  void CS2::receiver() {
    xlog("CS2 receiver started");
    struct sockaddr_in sockaddr_in;
    int sock;
    sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), "0.0.0.0", CS2_PORT_RECV);
    if (sock < 0) {
      xlog("Unable to create UDP connection for receiving data from CS2");
      return;
    }

    if (bind(sock, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
      xlog("Unable to bind the socket for CS2 receiver");
    }
    char buffer[CS2_CMD_BUF_LEN];
    while(run) {
      //try to receive some data, this is a blocking call
      xlog("Receiver waiting for data");
      ssize_t datalen = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
      if (datalen <= 0) {
        xlog("Unable to receive data");
        close(sock);
        return;
      }
      else {
        xlog("Receiver %i bytes received", datalen);
        hexlog(buffer, datalen);
      }
    }
    close(sock);
    xlog("CS2 receiver ended");
  }

} // namespace

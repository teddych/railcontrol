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
	extern "C" CS2* create_cs2(struct Params& params) {
		return new CS2(params);
  }

	// delete instance of cs2
  extern "C" void destroy_cs2(CS2* cs2) {
    delete(cs2);
  }

  // start the thing
	CS2::CS2(struct Params& params) {
		std::stringstream ss;
		ss << "Maerklin Central Station 2 (CS2) / " << params.name;
		name = ss.str();
		run = true;
		senderThread = std::thread([this] {sender();});
	}

  // stop the thing
	CS2::~CS2() {
		run = false;
		senderThread.join();
	}

  // return the name
	std::string CS2::getName() const {
		return name;
  }

	// GO-command (turn on booster)
  void CS2::go() {
  }

	// Stop-command (turn off booster)
  void CS2::stop() {
  }

	/*
	// send a command to the CS2
	std::string CS2::sendCommand(const int sock, const struct sockaddr* sockaddr_in, const unsigned char prio, const unsigned char command, const unsigned char response, const unsigned char length, const char* data) {
    static unsigned short hash = 0x7337;
		char buffer[CS2_CMD_BUF_LEN];
		memset (buffer + 5, 0, CS2_CMD_BUF_LEN - 5);
    buffer[0] = (prio << 1) | (command >> 7);
    buffer[1] = (command << 1) | (response & 0x01);
    buffer[2] = (hash >> 8);
    buffer[3] = (hash & 0xff);
    buffer[4] = length;
		// copy 8 byte from data to buffer[5..12]
		int64_t* buffer_data = (int64_t*)(buffer + 5);
		*buffer_data = (int64_t)(*data);
		if (sendto(sock, buffer, sizeof (buffer), 0, sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
      return "Unable to send data to CS2";
    }
		return "";
	}
	*/

	void CS2::createCommandHeader(char* buffer, const unsigned char prio, const unsigned char command, const unsigned char response, const unsigned char length) {
		buffer[0] = (prio << 1) | (command >> 7);
		buffer[1] = (command << 1) | (response & 0x01);
		buffer[2] = (hash >> 8);
		buffer[3] = (hash & 0xFF);
		buffer[4] = length;
	}

	void CS2::createLocID(char* buffer, const protocol_t protocol, address_t address) {
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

	// set the speed of a loco
	void CS2::locoSpeed(protocol_t protocol, address_t address, speed_t speed) {
		char buffer[CS2_CMD_BUF_LEN];
		// fill up header & locid
		createCommandHeader(buffer, 0, 0x04, 0, 6);
		// set data buffer to 0
		int64_t* buffer_data = (int64_t*) (buffer + 5);
		*buffer_data = 0L;
		// set locID
		createLocID(buffer + 5, protocol, address);
		// set speed
		buffer[10] = (speed >> 8);
		buffer[11] = (speed & 0xFF);

//    hexlog(buffer, sizeof(buffer));
		if (sendto(senderSocket, buffer, sizeof(buffer), 0, (struct sockaddr*) &sockaddr_inSender, sizeof(struct sockaddr_in))
		    == -1) {
			xlog("Unable to send data to CS2");
		}

		/*
		 return sendCommand(clientSocket, )
		 std::stringstream ss;
		 ss << "Setting speed in CS2 of loco " << (unsigned int)protocol << "/" << address << " to speed " << speed;
		 std::string s = ss.str();
		 return s;
		 */
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
//    char buffer[CS2_CMD_BUF_LEN];
    while(run) {
//      //try to receive some data, this is a blocking call
//      xlog("Receiver waiting for data");
//      ssize_t datalen = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
//      if (datalen <= 0) {
//        xlog("Unable to receive data");
//        close(sock);
//        return;
//      }
//      else {
//        xlog("Receiver %i bytes received", datalen);
//        hexlog(buffer, datalen);
//      }
    }
    close(sock);
    xlog("CS2 receiver ended");
  }

  // the sender thread of the CS2
  void CS2::sender() {
    xlog("CS2 sender started");
    senderSocket = create_udp_connection((struct sockaddr*)&sockaddr_inSender, sizeof(struct sockaddr_in), CS2_IP, CS2_PORT_SEND);
    if (senderSocket < 0) {
//      xlog("Unable to create UDP connection for sending data to CS2");
      return;
    };

		while (run) {
			sleep(1);
		}

//    char buffer[CS2_CMD_BUF_LEN];
//    memset (buffer, 0, sizeof (buffer));
//    unsigned char prio = 0;
//    unsigned char command = 0;
//    unsigned char response = 0;
//		unsigned char length = 5;
//
//		/*
//    buffer[0] = (prio << 1) | (command >> 7);
//    buffer[1] = (command << 1) | (response & 0x01);
//    buffer[2] = (hash >> 8);
//    buffer[3] = (hash & 0xff);
//    buffer[4] = 5;
//		*/
//    buffer[9] = 1;
//		sendCommand(senderSocket, (struct sockaddr*)&sockaddr_in, prio, command, response, length, buffer + 5);
//		/*
//    //send the message
//    hexlog(buffer, sizeof(buffer));
//    if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
//      xlog("Unable to send data");
//    }
//		*/
//
//    command = 0x04;
//		length = 5;
//		/*
//    buffer[0] = (prio << 1) | (command >> 7);
//    buffer[1] = (command << 1) | (response & 0x01);
//    buffer[2] = (hash >> 8);
//    buffer[3] = (hash & 0xff);
//    buffer[4] = 6;
//		*/
//    buffer[7] = 0xc4;
//    buffer[8] = 0x68;
//    buffer[9] = 0;
//    buffer[10] = 0x50;
//		sendCommand(senderSocket, (struct sockaddr*)&sockaddr_in, prio, command, response, length, buffer + 5);
//
//    //send the message
//		/*
//    hexlog(buffer, sizeof(buffer));
//    if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_inSender, sizeof(struct sockaddr_inSender)) == -1) {
//      xlog("Unable to send data");
//    }
//		*/
//
//    sleep(1);
//    buffer[10] = 0x0;
//
//    //send the message
//    hexlog(buffer, sizeof(buffer));
//    if (sendto(senderSocket, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_inSender, sizeof(struct sockaddr_in)) == -1) {
//      xlog("Unable to send data");
//    }
//
//    sleep(1);
//
//    command = 0;
//    buffer[0] = (prio << 1) | (command >> 7);
//    buffer[1] = (command << 1) | (response & 0x01);
//    buffer[2] = (hash >> 8);
//    buffer[3] = (hash & 0xff);
//    buffer[4] = 5;
//    buffer[7] = 0;
//    buffer[8] = 0;
//    buffer[9] = 0;
//    buffer[10] = 0;
//    //send the message
//    hexlog(buffer, sizeof(buffer));
//    if (sendto(senderSocket, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
//      xlog("Unable to send data");
//    }
    close (senderSocket);
    xlog("CS2 sender ended");
  }

} // namespace

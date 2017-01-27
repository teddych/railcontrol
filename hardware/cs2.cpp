#include <arpa/inet.h>
#include <cstdint>    //int64_t;
#include <cstdio>     //printf
#include <cstdlib>    //exit(0);
#include <cstring>    //memset
#include <iostream>
#include <thread>
#include <unistd.h>   //close;

#include "../util.h"
#include "cs2.h"

#define CS2_IP "192.168.0.190"
#define CS2_BUFLEN 13		// Length of buffer
#define CS2_PORT_SEND 15731		//The port on which to send data
#define CS2_PORT_RECV 15730		//The port on which to receive data

namespace hardware {

	// create instance of cs2
  extern "C" cs2* create_cs2() {
    return new cs2();
  }

	// delete instance of cs2
  extern "C" void destroy_cs2(cs2* cs2) {
    delete(cs2);
  }

  // start the thing
  int cs2::start(struct params &params) {
    return 0;
  }

  // stop the thing
  int cs2::stop() {
    return 0;
  }

  // return the name
  std::string cs2::name() const {
    return "Maerklin Central Station 2 (CS2)";
  }

	// send a command to the CS2
	void cs2::send_command(const int sock, const struct sockaddr* sockaddr_in, const unsigned char prio, const unsigned char command, const unsigned char response, const unsigned char length, const char* data) {
    static unsigned short hash = 0x7337;
		char buffer[CS2_BUFLEN];
    memset (buffer + 5, 0, CS2_BUFLEN - 5);
    buffer[0] = (prio << 1) | (command >> 7);
    buffer[1] = (command << 1) | (response & 0x01);
    buffer[2] = (hash >> 8);
    buffer[3] = (hash & 0xff);
    buffer[4] = length;
		// copy 8 byte from data to buffer[5..12]
		int64_t* buffer_data = (int64_t*)(buffer + 5);
		*buffer_data = (int64_t)(*data);
		if (sendto(sock, buffer, sizeof (buffer), 0, sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
      xlog("Unable to send data to CS2");
    }
	}


	// set the speed of a loco
	void cs2::loco_speed(protocol_t protocol, address_t address, speed_t speed) {
		xlog("Setting speed of loco %u/%u to %u", protocol, address, speed);
	}

  // the receiver thread of the CS2
  void cs2::receiver() {
    xlog("Receiver started");
    struct sockaddr_in sockaddr_in;
    int sock;
    sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), "0.0.0.0", CS2_PORT_RECV);
    if (sock < 0) {
      xlog("Unable to create UDP connection for receiving data from CS2");
      return;
    }

    if (bind(sock, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
      xlog("Unable to bind the socket");
    }
    char buffer[CS2_BUFLEN];
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
    xlog("Receiver ended");
  }

  // the sender thread of the CS2
  void cs2::sender() {
    static unsigned short hash = 0x7337;
    xlog("Sender started");
    struct sockaddr_in sockaddr_in;
    int sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), CS2_IP, CS2_PORT_SEND);
    if (sock < 0) {
      xlog("Unable to create UDP connection for sending data to CS2");
      return;
    };

    char buffer[CS2_BUFLEN];
    memset (buffer, 0, sizeof (buffer));
    unsigned char prio = 0;
    unsigned char command = 0;
    unsigned char response = 0;
		unsigned char length = 5;

		/*
    buffer[0] = (prio << 1) | (command >> 7);
    buffer[1] = (command << 1) | (response & 0x01);
    buffer[2] = (hash >> 8);
    buffer[3] = (hash & 0xff);
    buffer[4] = 5;
		*/
    buffer[9] = 1;
		send_command(sock, (struct sockaddr*)&sockaddr_in, prio, command, response, length, buffer + 5);
		/*
    //send the message
    hexlog(buffer, sizeof(buffer));
    if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
      xlog("Unable to send data");
    }
		*/

    command = 0x04;
		length = 5;
		/*
    buffer[0] = (prio << 1) | (command >> 7);
    buffer[1] = (command << 1) | (response & 0x01);
    buffer[2] = (hash >> 8);
    buffer[3] = (hash & 0xff);
    buffer[4] = 6;
		*/
    buffer[7] = 0xc4;
    buffer[8] = 0x68;
    buffer[9] = 0;
    buffer[10] = 0x50;
		send_command(sock, (struct sockaddr*)&sockaddr_in, prio, command, response, length, buffer + 5);

    //send the message
		/*
    hexlog(buffer, sizeof(buffer));
    if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
      xlog("Unable to send data");
    }
		*/

    sleep(1);
    buffer[10] = 0x0;

    //send the message
    hexlog(buffer, sizeof(buffer));
    if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
      xlog("Unable to send data");
    }

    sleep(1);

    command = 0;
    buffer[0] = (prio << 1) | (command >> 7);
    buffer[1] = (command << 1) | (response & 0x01);
    buffer[2] = (hash >> 8);
    buffer[3] = (hash & 0xff);
    buffer[4] = 5;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10] = 0;
    //send the message
    hexlog(buffer, sizeof(buffer));
    if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
      xlog("Unable to send data");
    }
    close (sock);
    xlog("Sender ended");
  }

} // namespace

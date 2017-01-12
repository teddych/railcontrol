#include <iostream>
#include <cstdio>		//printf
#include <cstring>		//memset
#include <cstdlib>		//exit(0);
#include <unistd.h>		//close;
#include <arpa/inet.h>

#include "../util.h"
#include "cs2.h"

// return the name
std::string cs2::name() {
  return "Maerklin Central Station 2 (CS2)";
}

// the receiver thread of the CS2
void cs2::receiver() {
  xlog("Receiver started");
  struct sockaddr_in sockaddr_in;
  int sock;
  sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), "0.0.0.0", PORT_RECV);
  if (sock < 0) {
    xlog("Unable to create UDP connection for receiving data from CS2");
    return;
  }

  if (bind(sock, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
    xlog("Unable to bind the socket");
  }
  char buffer[BUFLEN];
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
  xlog("Sender started");
  struct sockaddr_in sockaddr_in;
  int sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), SERVER, PORT_SEND);
  if (sock < 0) {
    xlog("Unable to create UDP connection for sending data to CS2");
    return;
  };

  char buffer[BUFLEN];
  memset (buffer, 0, sizeof (buffer));
  unsigned char prio = 0;
  unsigned char command = 0;
  unsigned char response = 0;
  unsigned short hash = 0x7337;
  buffer[0] = (prio << 1) | (command >> 7);
  buffer[1] = (command << 1) | (response & 0x01);
  buffer[2] = (hash >> 8);
  buffer[3] = (hash & 0xff);
  buffer[4] = 5;
  buffer[9] = 1;

  //send the message
  hexlog(buffer, sizeof(buffer));
  if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
    xlog("Unable to send data");
  }

  command = 0x04;
  buffer[0] = (prio << 1) | (command >> 7);
  buffer[1] = (command << 1) | (response & 0x01);
  buffer[2] = (hash >> 8);
  buffer[3] = (hash & 0xff);
  buffer[4] = 6;
  buffer[7] = 0xc4;
  buffer[8] = 0x68;
  buffer[9] = 0;
  buffer[10] = 0x50;

  //send the message
  hexlog(buffer, sizeof(buffer));
  if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
    xlog("Unable to send data");
  }

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

extern "C" cs2* create_cs2() {
  return new cs2();
}

extern "C" void destroy_cs2(cs2* cs2) {
  delete(cs2);
}

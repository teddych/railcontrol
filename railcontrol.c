#include <stdio.h>		//printf
#include <string.h>		//memset
#include <stdlib.h>		//exit(0);
#include <unistd.h>		//close;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdarg.h>             // va_* in xlog

#define SERVER "192.168.0.190"
#define BUFLEN 13		// Length of buffer
#define PORT_SEND 15731		//The port on which to send data
#define PORT_RECV 15730		//The port on which to receive data
//#define PORT_RECV 1030		//The port on which to receive data

static volatile unsigned char run;


// create log text
void xlog(const char* logtext, ...) {
  char buffer[128];
  va_list ap;
  va_start(ap, logtext);
  vsnprintf(buffer, sizeof(buffer), logtext, ap);
  // we ignore text more then length of buffer
  // prevent reading more then the buffer size
  buffer[sizeof(buffer) - 1] = 0;

  printf("%s\n", buffer);
  fflush(stdout);
  va_end(ap);
}

// log data in hex format
void hexlog(const char* hex, const size_t size) {
    char buffer[128];
    int num = 16;
    int pos = 0;
    int bufpos = 0;
    memset(buffer, ' ', sizeof(buffer));
    while (pos < (int)size) {
        int hexpos = bufpos * 3 + 12;
        int asciipos = num * 3 + 15 + bufpos;
        if (pos % num >= num >> 1) {
            hexpos++;
            asciipos++;
        }
        if (bufpos == 0) {
            snprintf(buffer, sizeof(buffer), "0x%08x", pos);
            buffer[10] = ' ';
        }
        snprintf(buffer + hexpos, sizeof(buffer) - hexpos, " %02x", hex[pos] & 0xFF);
        buffer[hexpos + 3] = ' ';
        //buffer[hexpos + 4] = ' ';
        snprintf(buffer + asciipos, sizeof(buffer) - asciipos, "%c", (hex[pos] > 32 && hex[pos] <= 126 && hex[pos] != 37 ? hex[pos] : '.'));
        buffer[asciipos + 1] = ' ';
        buffer[asciipos + 2] = 0;
        pos++;
        bufpos++;
        if (pos % num == 0) {
            xlog(buffer);
            memset(buffer, ' ', sizeof(buffer));
            bufpos = 0;
        }
    }
    if (bufpos) {
        xlog(buffer);
    }
}

// create a UDP connection to a server on a port
int create_udp_connection(const struct sockaddr* sockaddr, const unsigned int sockaddr_len, const char* server, const unsigned short port) {
  int sock;

  if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    fprintf(stderr, "Unable to create UDP socket\n");
    return -1;
  }

  memset ((char*)sockaddr, 0, sockaddr_len);
  struct sockaddr_in* sockaddr_in = (struct sockaddr_in*)sockaddr;
  sockaddr_in->sin_family = AF_INET;
  sockaddr_in->sin_port = htons(port);
  if (inet_aton (server, &sockaddr_in->sin_addr) == 0) {
    fprintf(stderr, "inet_aton() failed\n");
    return -1;
  }
  return sock;
}

// the receiver thread of the CS2
void* cs2_receiver(void* params) {
  xlog("Receiver started");
  struct sockaddr_in sockaddr_in;
  int sock;
  sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), "0.0.0.0", PORT_RECV);
  if (sock < 0) {
    fprintf(stderr, "Unable to create UDP connection for receiving data from CS2\n");
    return NULL;
  }

  //int broadcast=1;
  //setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

  if (bind(sock, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
    xlog("Unable to bind the socket");
  }
  char buffer[BUFLEN];
  while(run) {
    //try to receive some data, this is a blocking call
    xlog("Receiver waiting for data");
    ssize_t datalen = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
    if (datalen <= 0) {
      fprintf(stderr, "recvfrom()");
      close(sock);
      return NULL;
    }
    else {
      xlog("Receiver %i bytes received", datalen);
      hexlog(buffer, datalen);
    }
  }
  close(sock);
  xlog("Receiver ended");
  return NULL;
}

// the sender thread of the CS2
void* cs2_sender(void* params) {
  xlog("Sender started");
  struct sockaddr_in sockaddr_in;
  int sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), SERVER, PORT_SEND);
  if (sock < 0) {
    fprintf(stderr, "Unable to create UDP connection for sending data to CS2\n");
    return NULL;
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
    fprintf(stderr, "sendto()");
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
    fprintf(stderr, "sendto()");
  }

  sleep(1);
  buffer[10] = 0x0;

  //send the message
  hexlog(buffer, sizeof(buffer));
  if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
    fprintf(stderr, "sendto()");
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
    fprintf(stderr, "sendto()");
  }
  close (sock);
  xlog("Sender ended");
  return NULL;
}

int main (int argc, char* argv[]) {
  xlog("OK");
  run = true;
  pthread_t thread_cs2_sender;
  pthread_t thread_cs2_receiver;
  pthread_create(&thread_cs2_receiver, NULL, cs2_receiver, NULL);
  pthread_create(&thread_cs2_sender, NULL, cs2_sender, NULL);
  pthread_join(thread_cs2_sender, NULL);
  run = false;
  pthread_join(thread_cs2_receiver, NULL);
//  sleep(2);
  return 0;
}



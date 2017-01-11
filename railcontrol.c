#include <stdio.h>		//printf
#include <string.h>		//memset
#include <stdlib.h>		//exit(0);
#include <unistd.h>		//close;
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>

#define SERVER "192.168.0.190"
#define BUFLEN 13		// Length of buffer
#define PORT_SEND 15731		//The port on which to send data
#define PORT_RECV 15730		//The port on which to receive data

static volatile unsigned char run;

void die (char *s) {
  perror (s);
  exit (1);
}

void hexlog(char* hex, size_t size) {
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
            printf("%s\n", buffer);
            memset(buffer, ' ', sizeof(buffer));
            bufpos = 0;
        }
    }
    if (bufpos) {
        printf("%s\n", buffer);
    }
}

int create_udp_connection(struct sockaddr* sockaddr, unsigned int sockaddr_len, char* server, unsigned short port) {
  int sock;

  if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die ("socket");
  }

  memset ((char*)sockaddr, 0, sockaddr_len);
  struct sockaddr_in* sockaddr_in = (struct sockaddr_in*)sockaddr;
  sockaddr_in->sin_family = AF_INET;
  sockaddr_in->sin_port = htons (port);

  if (inet_aton (server, &sockaddr_in->sin_addr) == 0) {
    fprintf (stderr, "inet_aton() failed\n");
    exit (1);
  }
  return sock;
}

void* cs2_receiver(void* params) {
  printf("Receiver started");
  struct sockaddr_in sockaddr_in;
  int sock;
  sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), SERVER, PORT_RECV);
  if (sock < 0) {
    die("create udp connection");
  };
  char buffer[BUFLEN];
  while(run) {
    //try to receive some data, this is a blocking call
    printf("Receiver waiting for data");
    if (recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL) == -1) {
      die("recvfrom()");
    }
    printf("Receiver data received");

    hexlog(buffer, sizeof(buffer));
  }
  close(sock);
  printf("Receiver ended");
  return NULL;
}

void* cs2_sender(void* params) {
  printf("Sender started");
  struct sockaddr_in sockaddr_in;
  int sock = create_udp_connection((struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in), SERVER, PORT_SEND);
  if (sock < 0) {
    die("create udp connection");
  };

  char buffer[BUFLEN];
  memset (buffer, 0, sizeof (buffer));
  unsigned char prio = 1;
  unsigned char command = 0;
  unsigned char response = 0;
  unsigned short hash = 0x7337;
  buffer[0] = (prio << 1) | (command >> 7);
  buffer[1] = (command << 1) | (response & 0x01);
  buffer[2] = (hash >> 8);
  buffer[3] = (hash & 0xff);
  buffer[4] = 5;

  //send the message
  if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
    die ("sendto()");
  }
  sleep(1);
  buffer[9] = 1;
  //send the message
  if (sendto(sock, buffer, sizeof (buffer), 0, (struct sockaddr*)&sockaddr_in, sizeof(struct sockaddr_in)) == -1) {
    die ("sendto()");
  }
  close (sock);
  printf("Sender ended");
  return NULL;
}

int main (void) {
  run = true;
  pthread_t thread_cs2_sender;
  pthread_t thread_cs2_receiver;
  pthread_create(&thread_cs2_receiver, NULL, cs2_receiver, NULL);
  pthread_create(&thread_cs2_sender, NULL, cs2_sender, NULL);
  pthread_join(thread_cs2_sender, NULL);
  run = false;
  pthread_join(thread_cs2_receiver, NULL);
  return 0;
}



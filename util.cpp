#include <arpa/inet.h>
#include <cstdarg>             // va_* in xlog
#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <iostream>
#include <sstream>
#include <unistd.h>		//close;

using std::cout;
using std::endl;
using std::stringstream;


// create log text
void xlog(const char* logtext, ...) {
  char buffer[128];
  va_list ap;
  va_start(ap, logtext);
  vsnprintf(buffer, sizeof(buffer), logtext, ap);
  // we ignore text more then length of buffer
  // prevent reading more then the buffer size
  buffer[sizeof(buffer) - 1] = 0;

  cout << buffer << endl;
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
    xlog("Unable to create UDP socket");
    return -1;
  }

  memset ((char*)sockaddr, 0, sockaddr_len);
  struct sockaddr_in* sockaddr_in = (struct sockaddr_in*)sockaddr;
  sockaddr_in->sin_family = AF_INET;
  sockaddr_in->sin_port = htons(port);
  if (inet_aton (server, &sockaddr_in->sin_addr) == 0) {
    xlog("inet_aton() failed");
    return -1;
  }
  return sock;
}


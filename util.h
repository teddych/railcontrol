#include <string>

// create log text
void xlog(const char* logtext, ...);

// log data in hex format
void hexlog(const char* hex, const size_t size);

// create a UDP connection to a server on a port
int create_udp_connection(const struct sockaddr* sockaddr, const unsigned int sockaddr_len, const char* server, const unsigned short port);


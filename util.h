#include <string>
#include <vector>

// replace string from with to in str
void str_replace(std::string& str, const std::string& from, const std::string& to);

// split string in vector<string>
void str_split(const std::string& str, const std::string& delimiter, std::vector<std::string>& list);

// create log text
void xlog(const char* logtext, ...);

// log data in hex format
void hexlog(const char* hex, const size_t size);

// create a UDP connection to a server on a port
int create_udp_connection(const struct sockaddr* sockaddr, const unsigned int sockaddr_len, const char* server, const unsigned short port);


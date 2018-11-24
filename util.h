#pragma once

#include <string>
#include <vector>
#include <map>

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

// receive with timeout
int recv_timeout(int sock, char* buf, const size_t buflen, const int flags);

// receive with timeout
int send_timeout(int sock, const char* buf, const size_t buflen, const int flags);

std::string GetStringMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const std::string& defaultValue = "");
int GetIntegerMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const int defaultValue = 0);
bool GetBoolMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const bool defaultValue = false);

std::string toStringWithLeadingZeros(const unsigned int number, const unsigned char chars);

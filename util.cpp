#include <arpa/inet.h>
#include <cstdarg>    // va_* in xlog
#include <cstdio>     // printf
#include <cstdlib>    // exit(0);
#include <cstring>    // memset
#include <iostream>   // cout
#include <sstream>
#include <sys/time.h> // gettimeofday
#include <unistd.h>   // close;

#include "util.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;

void str_replace(std::string& str, const std::string& from, const std::string& to) {
	while (true) {
		size_t start_pos = str.find(from);
		if (start_pos == string::npos) {
			return;
		}
		str.replace(start_pos, from.length(), to);
	}
}

void str_split(const std::string& str_in, const std::string &delimiter, std::vector<string> &list) {
	size_t length_delim = delimiter.length();
	string str(str_in);
	while (true) {
		size_t pos = str.find(delimiter);
		list.push_back(str.substr(0, pos));
		if (pos == string::npos) {
			return;
		}
		str = string(str.substr(pos + length_delim, string::npos));
	}
}

// create log text
void xlog(const char* logtext, ...) {
  char buffer[1024];
	struct timeval timestamp;
	struct tm tm;
  va_list ap;
  va_start(ap, logtext);

	// Get the current time
	gettimeofday(&timestamp, NULL);

	// Convert it to local time representation
	gmtime_r(&timestamp.tv_sec, &tm);
	strftime(buffer, sizeof(buffer), "%F %T.", &tm);

	snprintf(buffer + 20, sizeof(buffer) - 20, "%06li ", timestamp.tv_usec);

  vsnprintf(buffer + 27, sizeof(buffer) - 27, logtext, ap);
  // we ignore text that is longer then our buffer
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

	// create socket
	if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		xlog("Unable to create UDP socket");
		return -1;
	}

	// setting listening port
	memset ((char*)sockaddr, 0, sockaddr_len);
	struct sockaddr_in* sockaddr_in = (struct sockaddr_in*)sockaddr;
	sockaddr_in->sin_family = AF_INET;
	sockaddr_in->sin_port = htons(port);
	if (inet_aton (server, &sockaddr_in->sin_addr) == 0) {
		xlog("inet_aton() failed");
		return -1;
	}

	// setting receive timeout to 1s
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	return sock;
}

int recv_timeout(int sock, char* buf, const size_t buflen, const int flags) {
	errno = 0;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	int ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &timeout));
	if (ret < 0) {
		return ret;
	}
	if (ret == 0) {
		errno = ETIMEDOUT;
		return -1;
	}
	ret = recv(sock, buf, buflen, flags);
	if (ret <= 0) {
		errno = ECONNRESET;
		return -1;
	}
	return ret;
}

int send_timeout(int sock, const char* buf, const size_t buflen, const int flags) {
	errno = 0;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	int ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, NULL, &set, NULL, &timeout));
	if (ret < 0) {
		return ret;
	}
	if (ret == 0) {
		errno = ETIMEDOUT;
		return -1;
	}
	ret = send(sock, buf, buflen, flags | MSG_NOSIGNAL);
	if (ret <= 0) {
		errno = ECONNRESET;
		return -1;
	}
	return ret;
}

std::string GetStringMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const std::string& defaultValue)
{
	if (map.count(key) == 0)
	{
		return defaultValue;
	}
	return map.at(key);
}

int GetIntegerMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const int defaultValue)
{
	if (map.count(key) == 0)
	{
		return defaultValue;
	}
	return stoi(map.at(key));
}

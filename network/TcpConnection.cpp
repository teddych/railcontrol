#include <arpa/inet.h>
#include <unistd.h>   // close & TEMP_FAILURE_RETRY;

#include "network/TcpConnection.h"
#include "util.h"

namespace Network
{
	TcpConnection::TcpConnection(int socket) :
		connectionSocket(socket),
		connected(true)
	{
		xlog("TCP connection established to ");
	}

	TcpConnection::~TcpConnection()
	{
		Terminate();
	}

	void TcpConnection::Terminate()
	{
		if (connected)
		{
			connected = false;
			close(connectionSocket);
		}
	}

	int TcpConnection::Send(const char* buf, const size_t buflen, const int flags) {
		errno = 0;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(connectionSocket, &set);
		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		int ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, NULL, &set, NULL, &timeout));
		if (ret < 0)
		{
			return ret;
		}
		if (ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
		ret = send(connectionSocket, buf, buflen, flags | MSG_NOSIGNAL);
		if (ret <= 0)
		{
			errno = ECONNRESET;
			return -1;
		}
		return ret;
	}

	int TcpConnection::Send(const std::string& string, const int flags)
	{
		return Send(string.c_str(), string.size(), flags);
	}

	int TcpConnection::Send(const std::string& string)
	{
		return Send(string, 0);
	}

	int TcpConnection::Receive(char* buf, const size_t buflen, const int flags)
	{
		errno = 0;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(connectionSocket, &set);
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &timeout));
		if (ret < 0)
		{
			return ret;
		}
		if (ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
		ret = recv(connectionSocket, buf, buflen, flags);
		if (ret <= 0)
		{
			errno = ECONNRESET;
			return -1;
		}
		return ret;
	}
}

#pragma once

#include <string>

namespace Network
{
	class TcpConnection
	{
		public:
			TcpConnection(int socket)
			:	connectionSocket(socket),
				connected(true)
			{}

			~TcpConnection()
			{
				Terminate();
			}

			void Terminate();
			int Send(const char* buf, const size_t buflen, const int flags);
			int Send(const std::string& string, const int flags)
			{
				return Send(string.c_str(), string.size(), flags);
			}

			int Send(const std::string& string)
			{
				return Send(string, 0);
			}

			int Receive(char* buf, const size_t buflen, const int flags);

		private:
			int connectionSocket;
			volatile bool connected;
	};
}

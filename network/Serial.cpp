#include <fcntl.h>
#include <termios.h>

#include "network/Select.h"
#include "network/Serial.h"

namespace Network
{
	void Serial::Init()
	{
		fileHandle = open(tty.c_str(), O_RDWR | O_NOCTTY);
		if (fileHandle == -1)
		{
			logger->Error("Unable to open serial {0}", tty);
			return;
		}

		struct termios options;
		tcgetattr(fileHandle, &options);
		options.c_cflag = 0;
		options.c_cc[VMIN] = 1;     // read one byte at least
		options.c_cc[VTIME] = 0;    // timeout disabled
		options.c_lflag = 0;
		options.c_iflag = 0;
		options.c_oflag = 0;
		cfsetispeed(&options, dataSpeed);
		cfsetospeed(&options, dataSpeed);
		switch (dataBits)
		{
			case 5:
				options.c_cflag |= CS5;     // 8 data bits
				break;

			case 6:
				options.c_cflag |= CS6;     // 8 data bits
				break;

			case 7:
				options.c_cflag |= CS7;     // 8 data bits
				break;

			case 8:
			default:
				options.c_cflag |= CS8;     // 8 data bits
				break;
		}

		if (stopBits == 2)
		{
			options.c_cflag |= CSTOPB; // 2 stop bit
		}
		// else 1 stop bit

		switch (parity)
		{
			case 'E':
			case 'e':
				options.c_cflag |= PARENB; // even parity
				break;

			case 'O':
			case 'o':
				options.c_cflag |= PARENB;
				options.c_cflag |= PARODD; // odd parity
				break;

			// default: no parity
		}

		// CSIZE not set: no datasize
		options.c_cflag |= CRTSCTS;  // hardware flow control
		options.c_cflag |= CLOCAL;  // ignore control lines
		options.c_cflag |= CREAD;   // enable receiver
		tcsetattr(fileHandle, TCSANOW, &options); // store options

		ClearBuffers();
	}

	bool Serial::Receive(std::string& data, const size_t maxData, const unsigned int timeout)
	{
		char dataBuffer[maxData];
		int ret = Receive(dataBuffer, maxData, timeout);
		if (ret < 0)
		{
			return false;
		}
		data.append(dataBuffer, ret);
		return true;
	}

	size_t Serial::Receive(char* data, const size_t maxData, const unsigned int timeout)
	{
		if (!IsConnected())
		{
			return -1;
		}
		std::lock_guard<std::mutex> Guard(fileHandleMutex);
		fd_set set;
		FD_ZERO(&set);
		FD_SET(fileHandle, &set);
		struct timeval tvTimeout;
		tvTimeout.tv_sec = timeout;
		tvTimeout.tv_usec = 0;

		size_t ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tvTimeout));
		if (ret <= 0)
		{
			return -1;
		}
		ret = read(fileHandle, data, maxData);
		if (ret <= 0)
		{
			return -1;
		}
		return ret;
	}

	bool Serial::ReceiveExact(std::string& data, const size_t length, const unsigned int timeout)
	{
		size_t startSize = data.length();
		size_t endSize = startSize + length;
		while (endSize > data.length())
		{
			bool ret = Receive(data, endSize - data.length(), timeout);
			if (ret == false)
			{
				return false;
			}
		}
		return true;
	}

	size_t Serial::ReceiveExact(char* data, const size_t length, const unsigned int timeout)
	{
		size_t actualSize = 0;
		size_t endSize = length;
		while (actualSize < endSize)
		{
			size_t ret = Receive(data + actualSize, endSize - actualSize, timeout);
			if (ret <= 0)
			{
				return actualSize;
			}
			actualSize += ret;
		}
		return actualSize;
	}
}

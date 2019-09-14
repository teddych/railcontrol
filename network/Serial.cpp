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
		cfsetispeed(&options, dataSpeed);
		cfsetospeed(&options, dataSpeed);
		options.c_cflag = 0;
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
		options.c_cc[VMIN] = 1;     // read one byte at least
		options.c_cc[VTIME] = 10;    // timeout = 0.1s
		tcsetattr(fileHandle, TCSANOW, &options); // store options

		ClearBuffers();
	}

	bool Serial::Receive(std::string& data, const unsigned int maxData, const unsigned int timeout)
	{
		if (!IsConnected())
		{
			logger->Error("Unable to receive from serial line. Not connected.");
			return false;
		}
		std::lock_guard<std::mutex> Guard(fileHandleMutex);
		fd_set set;
		FD_ZERO(&set);
		FD_SET(fileHandle, &set);
		struct timeval tvTimeout;
		tvTimeout.tv_sec = timeout;
		tvTimeout.tv_usec = 0;

		int ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tvTimeout));
		if (ret < 0)
		{
			logger->Error("Unable to receive from serial line (select).");
			ReInit();
			return false;
		}
		if (ret == 0)
		{
			logger->Error("Unable to receive from serial line. Timeout.");
			return false;
		}
		char dataBuffer[maxData];
		ret = read(fileHandle, &dataBuffer, maxData);
		if (ret <= 0)
		{
			logger->Error("Unable to receive from serial line (read).");
			ReInit();
			return false;
		}
		data.append(dataBuffer, ret);
		return true;
	}
}

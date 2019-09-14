#pragma once

#include <mutex>
#include <string>
#include <termios.h>
#include <unistd.h>   //close & write;

#include "Logger/Logger.h"

namespace Network
{
	class Serial
	{
		public:
			Serial(Logger::Logger* logger,
				const std::string& tty,
				const unsigned int dataSpeed, // from termio (ex. B9600)
				const unsigned char dataBits,
				const char parity,
				const unsigned char stopBits)
			:	logger(logger),
			 	tty(tty),
			 	dataSpeed(dataSpeed),
			 	dataBits(dataBits),
			 	parity(parity),
			 	stopBits(stopBits)
			{
				Init();
			}

			~Serial()
			{
				Close();
			}

			void ReInit() { Close(); Init(); }

			bool IsConnected() const { return fileHandle != -1; }

			void ClearBuffers() { tcflush(fileHandle, TCIOFLUSH); }

			unsigned int Send(const std::string& data)
			{
				std::lock_guard<std::mutex> Guard(fileHandleMutex);
				return write(fileHandle, data.c_str(), data.length());
			}

			unsigned int Send(const unsigned char data)
			{
				std::lock_guard<std::mutex> Guard(fileHandleMutex);
				return write(fileHandle, &data, 1);
			}

			unsigned int Send(const unsigned char* data, const size_t size)
			{
				std::lock_guard<std::mutex> Guard(fileHandleMutex);
				return write(fileHandle, data, size);
			}

			bool Receive(std::string& data, const unsigned int maxData = 1024, const unsigned int timeout = 1);

		private:
			void Init();
			void Close() { if (fileHandle == -1) return; close(fileHandle); fileHandle = -1; }

			Logger::Logger* logger;
			const std::string tty;
			const unsigned int dataSpeed;
			const unsigned char dataBits;
			const char parity;
			const unsigned char stopBits;
			int fileHandle;
			mutable std::mutex fileHandleMutex;
	};
}

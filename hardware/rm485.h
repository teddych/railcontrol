#pragma once

#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>   //close & write;


#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "Logger/Logger.h"
#include "manager.h"

namespace hardware
{
	class RM485 : HardwareInterface
	{
		private:
			class CRC8
			{
				public:
					static uint8_t calcString(const uint8_t* const s, const uint8_t size);

					CRC8() : actualValue(0x07) {};

					void calcChar(const uint8_t c) { actualValue = crcTable[actualValue ^ c]; }
					uint8_t value() const { return actualValue; }

				private:
					static const uint8_t crcTable[256];
					uint8_t actualValue;
			};

			class RS485
			{
				public:
					RS485(const std::string& tty);
					~RS485();
					bool Send(const uint8_t address, const uint8_t command, const uint8_t* c = nullptr, const size_t length = 0);
					size_t Receive(uint8_t* c, const size_t length);

				private:
					int WriteChar(uint8_t c);
					int ReadChar(uint8_t& c);

					int fileDescriptor;

					static const uint8_t StartMessage = 0xA5;
			};

			class Communication
			{
				public:
					static const uint8_t MaxInputBytesPerModule = 16;
				
					Communication(const std::string& tty) : rs485(tty) {}

					uint8_t Version(const uint8_t address);
					ssize_t ReadAll(const uint8_t address, uint8_t* data);
					ssize_t ReadDelta(const uint8_t address, uint8_t* addresses, uint8_t* data);

				private:
					static const uint8_t MaxDeltaPerMessage = 32;
					static const uint8_t MinMessageLength = 5; // Laenge einer Message ohne Daten

					enum Commands : uint8_t
					{
						CommandVersion = 0x01,
						CommandReadAll = 0x02,
						CommandReadDelta = 0x03,
						CommandChangeAddress = 0x04, // unused but reserved
						CommandNumberOfModules = 0x05 // unused but reserved
					};

					RS485 rs485;
			};

		public:
			RM485(const HardwareParams* params);
			~RM485();
			const std::string GetName() const override { return name; };

			bool CanHandleFeedback() const { return true; }

			void GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const override
			{
				argumentTypes[1] = SerialPort;
			}

		private:
			Logger::Logger* logger;
			std::string name;
			Manager* manager;
			int ttyFileDescriptor;
			Communication communication;
			volatile bool run;
			std::thread rm485Thread;

			static const uint8_t NrOfModules = 32;
			bool rmAlive[NrOfModules];
			uint8_t data[NrOfModules * Communication::MaxInputBytesPerModule];

			uint8_t rescanAddress;
			uint8_t rescanCount;
			static const uint8_t RescanCountStart = 10;

			void ScanAddress(uint16_t address);
			void ScanBus();
			void ReadInitData();
			void ReadUpdateData();
			void RM485Worker();
	};

	extern "C" RM485* create_rm485(const HardwareParams* params);
	extern "C" void destroy_rm485(RM485* RM485);
} // namespace

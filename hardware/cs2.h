#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <thread>

#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "manager.h"

// CAN protocol specification at http://streaming.maerklin.de/public-media/cs2/cs2CAN-Protokoll-2_0.pdf

namespace hardware
{
	class CS2 : HardwareInterface
	{
		public:
			CS2(const HardwareParams* params);
			~CS2();
			const std::string GetName() const override { return name; };
			void GetProtocols(std::vector<protocol_t>& protocols) const override;
			virtual bool ProtocolSupported(protocol_t protocol) const override;
			void Booster(const boosterStatus_t status) override;
			void SetLocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed) override;
			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;
			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			volatile unsigned char run;
			std::string name;
			Manager* manager;
			struct sockaddr_in sockaddr_inSender;
			int senderSocket;
			std::thread receiverThread;
			static const unsigned short hash = 0x7337;
			typedef unsigned char cs2Prio_t;
			typedef unsigned char cs2Command_t;
			typedef unsigned char cs2Response_t;
			typedef unsigned char cs2Length_t;
			typedef uint32_t cs2Address_t;

			void intToData(const uint32_t i, char* buffer);
			uint32_t dataToInt(const char* buffer);
			uint16_t dataToShort(const char* buffer);
			void createCommandHeader(char* buffer, const cs2Prio_t prio, const cs2Command_t command, const cs2Response_t response, const cs2Length_t length);
			void readCommandHeader(char* buffer, cs2Prio_t& prio, cs2Command_t& command, cs2Response_t& response, cs2Length_t& length, cs2Address_t& address, protocol_t& protocol);
			void createLocID(char* buffer, const protocol_t& protocol, const address_t& address);
			void createAccessoryID(char* buffer, const protocol_t& protocol, const address_t& address);
			void receiver();
	};

	extern "C" CS2* create_cs2(const HardwareParams* params);
	extern "C" void destroy_cs2(CS2* cs2);

} // namespace


#pragma once

#include <cstring>
#include <string>
#include <thread>

#include "hardware_interface.h"
#include "hardware_params.h"
#include "manager.h"

namespace hardware {

	class CS2: HardwareInterface {
		public:
			CS2(const HardwareParams* params);
			~CS2();
			const std::string getName() const override { return name; };
			void getProtocols(std::vector<protocol_t>& protocols) const override;
			virtual bool protocolSupported(protocol_t protocol) const override;
			void booster(const boosterStatus_t status) override;
			void locoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) override;
			void locoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;
			void locoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
			void accessory(const protocol_t protocol, const address_t address, const accessoryState_t state) override;

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

			void createCommandHeader(char* buffer, const cs2Prio_t prio, const cs2Command_t command, const cs2Response_t response, const cs2Length_t length);
			void readCommandHeader(char* buffer, cs2Prio_t& prio, cs2Command_t& command, cs2Response_t& response, cs2Length_t& length);
			void createLocID(char* buffer, const protocol_t& protocol, const address_t& address);
			void receiver();
	};

} // namespace


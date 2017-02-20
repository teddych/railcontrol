#ifndef HARDWARE_CS2_H
#define HARDWARE_CS2_H

#include <cstring>
#include <string>
#include <thread>

#include "hardware_interface.h"
#include "hardware_params.h"

namespace hardware {

	typedef unsigned char cs2Prio_t;
	typedef unsigned char cs2Command_t;
	typedef unsigned char cs2Response_t;
	typedef unsigned char cs2Length_t;

	class CS2: HardwareInterface {
		public:
			CS2(const HardwareParams* params);
			~CS2();
			std::string getName() const override;
			void go() override;
			void stop() override;
			void locoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) override;
			void locoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
		private:
			void createCommandHeader(char* buffer, const cs2Prio_t& prio, const cs2Command_t& command, const cs2Response_t& response, const cs2Length_t& length);
			void createLocID(char* buffer, const protocol_t& protocol, const address_t& address);
			void receiver();
			volatile unsigned char run;
			std::string name;
			struct sockaddr_in sockaddr_inSender;
			int senderSocket;
			std::thread receiverThread;
			static const unsigned short hash = 0x7337;
	};

} // namespace

#endif // HARDWARE_CS2_H


#pragma once

#include <cstring>

#include "hardware_interface.h"
#include "hardware_params.h"

namespace hardware {

	class Virtual : HardwareInterface {
		public:
			// Constructor
			Virtual(const HardwareParams* params);

			// name() must be implemented
			const std::string getName() const override { return name; };

			// get available protocols of this control
			void getProtocols(std::vector<protocol_t>& protocols) const override;

			// is given protocol supported
			bool protocolSupported(protocol_t protocol) const override;

			// turn booster on or off
			void booster(const boosterStatus_t status) override;

			// set loco speed
			void locoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) override;

			// set loco direction
			void locoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;

			// set loco function
			void locoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;

			// accessory command
			void accessory(const protocol_t protocol, const address_t address, const accessoryState_t state) override;

		private:
			std::string name;
	};

} // namespace


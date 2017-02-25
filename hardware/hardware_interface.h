#ifndef HARDWARE_HARDAWRE_INTERFACE_H
#define HARDWARE_HARDWARE_INTERFACE_H

#include <string>

#include "../datatypes.h"

namespace hardware {

	class HardwareInterface {
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			HardwareInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~HardwareInterface() {};

			// get the name of the hardware
			virtual std::string getName() const = 0;

			// turn booster on or off
			virtual void booster(const boosterStatus_t status) = 0;

			// set loco speed
			virtual void locoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) = 0;

			// set loco direction
			virtual void locoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) = 0;

			// set loco function
			virtual void locoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) = 0;

			// accessory command
			virtual void accessory(const protocol_t protocol, const address_t address, const accessoryState_t state) = 0;
	};

} // namespace


#endif // HARDWARE_HARDWARE_INTERFACE_H


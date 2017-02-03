#ifndef HARDWARE_CONTROL_INTERFACE_H
#define HARDWARE_CONTROL_INTERFACE_H

#include <string>

#include "../control.h"

namespace hardware {

	class ControlInterface {
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			ControlInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~ControlInterface() {};

			// get the name of the hardware
			virtual std::string getName() const = 0;

			// GO-command (turn on booster)
			virtual void go() = 0;

			// Stop-command (turn off booster)
			virtual void stop() = 0;

			// set loco speed
			virtual void locoSpeed(protocol_t protocol, address_t address, speed_t speed) = 0;
	};

} // namespace


#endif // HARDWARE_CONTROL_INTERFACE_H


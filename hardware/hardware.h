#ifndef HARDWARE_HARDWARE_H
#define HARDWARE_HARDWARE_H

#include <string>

#include "../control.h"

namespace hardware {

	struct params {
		std::string ip;
	};

	class hardware {
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			hardware() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~hardware() {};

			// start the needed threads to serve the hardware
			virtual int start(struct params &params);

			// stop the threads
			virtual int stop();

			// get the name of the hardware
			virtual std::string name() const = 0;

			// set the speed of a loco
			virtual void loco_speed(protocol_t protocol, address_t address, speed_t speed) {};
	};

  // start the thing
  inline int hardware::start(struct params &params) {
    return 0;
  }

  // stop the thing
  inline int hardware::stop() {
    return 0;
  }

} // namespace


#endif // HARDWARE_HARDWARE_H


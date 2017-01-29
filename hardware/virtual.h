#ifndef HARDWARE_VIRTUAL_H
#define HARDWARE_VIRTUAL_H

#include <cstring>

#include "control_interface.h"

namespace hardware {

  class Virtual : ControlInterface {
    public:
			// Constructor
			Virtual(std::string& name2);

			// name() must be implemented
			std::string getName() const override;

		  // All possible methods that can be implemented
			// if not the method of the abstract class hardware is used

			// This method is called at startup to initialize the control
      //int start(struct params &params) override;

			// this method is called at shutdown to clean up the control
      //int stop() override;

			std::string locoSpeed(protocol_t protocol, address_t address,
			    speed_t speed) override;
			private:
			std::string name;
  };

} // namespace

#endif // HARDWARE_VIRTUAL_H


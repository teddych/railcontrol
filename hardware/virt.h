#ifndef HARDWARE_VIRTUAL_H
#define HARDWARE_VIRTUAL_H

#include <cstring>

#include "hardware.h"

namespace hardware {

  class virt : hardware {
    public:
			// name() must be implemented
      std::string name() const override;

		  // All possible methods that can be implemented
			// if not the method of the abstract class hardware is used

			// This method is called at startup to initialize the control
      //int start(struct params &params) override;

			// this method is called at shutdown to clean up the control
      //int stop() override;
  };

} // namespace

#endif // HARDWARE_VIRTUAL_H


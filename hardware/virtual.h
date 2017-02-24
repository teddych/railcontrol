#ifndef HARDWARE_VIRTUAL_H
#define HARDWARE_VIRTUAL_H

#include <cstring>

#include "hardware_interface.h"
#include "hardware_params.h"

namespace hardware {

  class Virtual : HardwareInterface {
    public:
			// Constructor
			Virtual(const HardwareParams* params);

			// name() must be implemented
			std::string getName() const override;

		  // All possible methods that can be implemented
			// if not the method of the abstract class hardware is used

			// GO-command (turn on booster)
      void go() override;

			// Stop-command (turn off booster)
      void stop() override;

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

#endif // HARDWARE_VIRTUAL_H


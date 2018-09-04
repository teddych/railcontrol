#pragma once

#include <string>
#include <vector>

#include "datatypes.h"

namespace hardware
{

	class HardwareInterface
	{
		public:
			// non virtual default constructor is needed to prevent polymorphism
			HardwareInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~HardwareInterface() {};

			// get the name of the hardware
			virtual const std::string GetName() const = 0;

			// get available protocols of this control
			virtual void GetProtocols(std::vector<protocol_t>& protocols) const = 0;

			// is given protocol supported
			virtual bool ProtocolSupported(protocol_t protocol) const = 0;

			// turn booster on or off
			virtual void Booster(const boosterStatus_t status) = 0;

			// set loco speed
			virtual void LocoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) = 0;

			// set loco direction
			virtual void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) = 0;

			// set loco function
			virtual void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) = 0;

			// accessory command
			virtual void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state) = 0;
	};

} // namespace


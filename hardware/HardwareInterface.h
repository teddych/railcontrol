#pragma once

#include <map>
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

			// can this control handle locos
			virtual bool CanHandleLocos() const { return false; }

			// can this control handle accessories, switches, ...
			virtual bool CanHandleAccessories() const { return false; }

			// can this control handle feedback
			virtual bool CanHandleFeedback() const { return false; }

			// get available loco protocols of this control
			virtual void GetLocoProtocols(std::vector<protocol_t>& protocols) const { protocols.push_back(ProtocolNone); }

			// is given loco protocol supported
			virtual bool LocoProtocolSupported(protocol_t protocol) const { return false; }

			// get available accessory protocols of this control
			virtual void GetAccessoryProtocols(std::vector<protocol_t>& protocols) const { protocols.push_back(ProtocolNone); }

			// is given accessory protocol supported
			virtual bool AccessoryProtocolSupported(protocol_t protocol) const { return false; }

			// get types of needed arguments of this control
			virtual void GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const {}

			// turn booster on or off
			virtual void Booster(const boosterStatus_t status) {};

			// set loco speed
			virtual void SetLocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed) {};

			// set loco direction
			virtual void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) {};

			// set loco function
			virtual void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) {};

			// accessory command
			virtual void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) {};
	};

} // namespace


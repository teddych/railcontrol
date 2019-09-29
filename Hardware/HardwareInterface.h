#pragma once

#include <map>
#include <string>
#include <vector>

#include "datatypes.h"
#include "Manager.h"

namespace Hardware
{

	class HardwareInterface
	{
		public:
			// non virtual default constructor is needed to prevent polymorphism
			HardwareInterface(Manager* manager, const controlID_t controlID, const std::string& name)
			:	manager(manager),
			 	controlID(controlID),
			 	name(name)
			{};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~HardwareInterface() {};

			// get the name of the hardware
			const std::string GetName() const { return name; }

			// can this control handle locos
			virtual bool CanHandleLocos() const { return false; }

			// can this control handle accessories, switches, ...
			virtual bool CanHandleAccessories() const { return false; }

			// can this control handle feedback
			virtual bool CanHandleFeedback() const { return false; }

			// get available loco protocols of this control
			virtual void GetLocoProtocols(std::vector<protocol_t>& protocols) const { protocols.push_back(ProtocolNone); }

			// is given loco protocol supported
			virtual bool LocoProtocolSupported(__attribute__((unused))  const protocol_t protocol) const { return false; }

			// get available accessory protocols of this control
			virtual void GetAccessoryProtocols(__attribute__((unused)) std::vector<protocol_t>& protocols) const { protocols.push_back(ProtocolNone); }

			// is given accessory protocol supported
			virtual bool AccessoryProtocolSupported(__attribute__((unused)) const protocol_t protocol) const { return false; }

			// get types of needed arguments of this control
			virtual void GetArgumentTypes(__attribute__((unused)) std::map<unsigned char,argumentType_t>& argumentTypes) const {}

			// turn booster on or off
			virtual void Booster(__attribute__((unused)) const boosterState_t status) {};

			// set loco speed
			virtual void LocoSpeed(__attribute__((unused)) const protocol_t& protocol, __attribute__((unused)) const address_t& address, __attribute__((unused)) const locoSpeed_t& speed) {};

			// set loco direction
			virtual void LocoDirection(__attribute__((unused)) const protocol_t& protocol, __attribute__((unused)) const address_t& address, __attribute__((unused)) const direction_t& direction) {};

			// set loco function
			virtual void LocoFunction(__attribute__((unused)) const protocol_t protocol, __attribute__((unused)) const address_t address, __attribute__((unused)) const function_t function, __attribute__((unused)) const bool on) {};

			// accessory command
			virtual void Accessory(__attribute__((unused)) const protocol_t protocol, __attribute__((unused)) const address_t address, __attribute__((unused)) const accessoryState_t state, __attribute__((unused)) const bool on) {};

		protected:
			Manager* manager;
			const controlID_t controlID;
			const std::string name;
	};

} // namespace


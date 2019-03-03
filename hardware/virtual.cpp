#include <sstream>
#include <string>

#include "hardware/virtual.h"
#include "manager.h"
#include "text/converters.h"
#include "util.h"

namespace hardware
{

	// create_virt and destroy_virt are used to instantiate
	// and delete the command station in main program

	// create instance of virtual
	extern "C" Virtual* create_virtual(const HardwareParams* params)
	{
		return new Virtual(params);
	}

	// delete instance of virtual
	extern "C" void destroy_virtual(Virtual* virt)
	{
		delete(virt);
	}


	Virtual::Virtual(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "Virtual Command Station / " + params->name),
	 	logger(Logger::Logger::GetLogger("Virtual " + params->name))
	{}

	// turn booster on or off
	void Virtual::Booster(const boosterStatus_t status)
	{
		logger->Info("Turning booster {0}", status ? "on" : "off");
	}

	// set loco speed
	void Virtual::SetLocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed)
	{
		logger->Info("Setting speed of loco {0}/{1} to speed {2}", protocol, address, speed);
	}

	// set the direction of a loco
	void Virtual::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		logger->Info("Setting direction of loco {0}/{1} to {2}", protocol, address, direction == DirectionRight ? "forward" : "reverse");
	}

	// set loco function
	void Virtual::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		logger->Info("Setting f%i of loco {0}/{1} to \"{2}\"", static_cast<int>(function), static_cast<int>(protocol), static_cast<int>(address), on ? "on" : "off");
	}

	// accessory command
	void Virtual::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		text::Converters::accessoryStatus(state, stateText);
		logger->Info("Setting state of virtual accessory {0}/{1}/{2} to \"{3}\"", static_cast<int>(protocol), static_cast<int>(address), stateText, on ? "on" : "off");
	}

} // namespace

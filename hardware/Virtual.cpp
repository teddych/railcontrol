#include <sstream>
#include <string>

#include "text/converters.h"
#include "hardware/Virtual.h"
#include "manager.h"
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
	{
		std::stringstream ss;
		ss << "Virtual Command Station / " << params->name;
		name = ss.str();
	}

	// turn booster on or off
	void Virtual::Booster(const boosterStatus_t status)
	{
		if (status)
		{
			xlog("Turning virtual booster on");
		}
		else
		{
			xlog("Turning virtual booster off");
		}
	}

	void Virtual::GetProtocols(std::vector<protocol_t>& protocols) const
	{
		protocols.push_back(ProtocolServer);
	}

	bool Virtual::ProtocolSupported(protocol_t protocol) const
	{
		return (protocol == ProtocolServer);
	}

	// set loco speed
	void Virtual::LocoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed)
	{
		xlog("Setting speed of virtual loco %i/%i to speed %i", protocol, address, speed);
	}

	// set the direction of a loco
	void Virtual::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		xlog("Setting direction of virtual loco %i/%i to %s", protocol, address, direction ? "forward" : "reverse");
	}

	// set loco function
	void Virtual::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		xlog("Setting f%i of virtual loco %i/%i to \"%s\"", (int)function, (int)protocol, (int)address, on ? "on" : "off");
	}

	// accessory command
	void Virtual::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state)
	{
		std::string colorText;
		std::string onText;
		text::Converters::accessoryStatus(state, colorText, onText);
		xlog("Setting state of virtual accessory %i/%i/%s to \"%s\"", (int)protocol, (int)address, colorText, onText);
	}

} // namespace

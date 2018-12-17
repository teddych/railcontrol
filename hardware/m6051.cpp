#include <cstring>    //memset
#include <sstream>
#include <unistd.h>   //close;

#include "text/converters.h"
#include "hardware/m6051.h"
#include "util.h"

namespace hardware
{

	// create instance of m6051
	extern "C" M6051* create_m6051(const HardwareParams* params)
	{
		return new M6051(params);
	}

	// delete instance of m6051
	extern "C" void destroy_m6051(M6051* m6051)
	{
		delete(m6051);
	}

	// start the thing
	M6051::M6051(const HardwareParams* params) :
		manager(params->manager)
	{
		std::stringstream ss;
		ss << "Maerklin Interface (6050/6051) / " << params->name;
		name = ss.str();
		xlog(name.c_str());
	}

	// stop the thing
	M6051::~M6051()
	{
	}

	void M6051::GetProtocols(std::vector<protocol_t>& protocols) const
	{
		protocols.push_back(ProtocolMM2);
	}

	bool M6051::ProtocolSupported(protocol_t protocol) const
	{
		return (protocol == ProtocolMM2);
	}

	// turn booster on or off
	void M6051::Booster(const boosterStatus_t status)
	{
		if (status)
		{
			xlog("Turning Märklin Interface booster on");
		}
		else
		{
			xlog("Turning Märklin Interface booster off");
		}
	}

	// set the speed of a loco
	void M6051::LocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed)
	{
		xlog("Setting speed of Märklin Interface loco %i/%i to speed %i", protocol, address, speed);
	}

	// set the direction of a loco
	void M6051::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		xlog("Setting direction of Märklin Interface loco %i/%i to %s", protocol, address, direction ? "forward" : "reverse");
	}

	// set loco function
	void M6051::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		xlog("Setting f%i of Märklin Interface loco %i/%i to \"%s\"", (int)function, (int)protocol, (int)address, on ? "on" : "off");
	}

	void M6051::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		text::Converters::accessoryStatus(state, stateText);
		xlog("Setting state of Märklin Interface accessory %i/%i/%s to \"%s\"", (int)protocol, (int)address, stateText.c_str(), on ? "on" : "off");
	}
} // namespace

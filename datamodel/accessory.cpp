#include <map>
#include <sstream>

#include "datamodel/accessory.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Accessory::Accessory(const accessoryID_t accessoryID,
		const std::string& name,
		const layoutPosition_t x,
		const layoutPosition_t y,
		const layoutPosition_t z,
		const layoutRotation_t rotation,
		const controlID_t controlID,
		const protocol_t protocol,
		const address_t address,
		const accessoryType_t type,
		const accessoryTimeout_t timeout,
		const bool inverted)
	:	LayoutItem(accessoryID, name, x, y, z, Width1, Height1, rotation),
		controlID(controlID),
		protocol(protocol),
		address(address),
		type(type),
		state(AccessoryStateOff),
		timeout(timeout),
		inverted(inverted)
	{
	}

	Accessory::Accessory(const std::string& serialized)
	{
		deserialize(serialized);
	}

	std::string Accessory::serialize() const
	{
		stringstream ss;
		ss << "objectType=Accessory;" << serializeWithoutType();
		return ss.str();
	}

	std::string Accessory::serializeWithoutType() const
	{
		stringstream ss;
		ss << LayoutItem::serialize()
			<< ";controlID=" << static_cast<int>(controlID)
			<< ";protocol=" << static_cast<int>(protocol)
			<< ";address=" << static_cast<int>(address)
			<< ";type=" << static_cast<int>(type)
			<< ";state=" << static_cast<int>(state)
			<< ";timeout=" << static_cast<int>(timeout)
			<< ";inverted=" << static_cast<int>(inverted);
		return ss.str();
	}

	bool Accessory::deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Accessory") == 0)
		{
			return deserialize(arguments);
		}
		return false;
	}

	bool Accessory::deserialize(const map<string,string>& arguments)
	{
		LayoutItem::deserialize(arguments);
		controlID = GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address = GetIntegerMapEntry(arguments, "address");
		type = GetIntegerMapEntry(arguments, "type");
		state = GetIntegerMapEntry(arguments, "state");
		timeout = GetIntegerMapEntry(arguments, "timeout", 100);
		inverted = GetBoolMapEntry(arguments, "inverted");
		return true;
	}
} // namespace datamodel

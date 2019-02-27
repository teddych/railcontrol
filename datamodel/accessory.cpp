#include <map>
#include <sstream>

#include "datamodel/accessory.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Accessory::Serialize() const
	{
		stringstream ss;
		ss << "objectType=Accessory;" << serializeWithoutType();
		return ss.str();
	}

	std::string Accessory::serializeWithoutType() const
	{
		stringstream ss;
		ss << LayoutItem::Serialize()
			<< ";controlID=" << static_cast<int>(controlID)
			<< ";protocol=" << static_cast<int>(protocol)
			<< ";address=" << static_cast<int>(address)
			<< ";type=" << static_cast<int>(type)
			<< ";state=" << static_cast<int>(state)
			<< ";timeout=" << static_cast<int>(timeout)
			<< ";inverted=" << static_cast<int>(inverted);
		return ss.str();
	}

	bool Accessory::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Accessory") == 0)
		{
			return Deserialize(arguments);
		}
		return false;
	}

	bool Accessory::Deserialize(const map<string,string>& arguments)
	{
		LayoutItem::Deserialize(arguments);
		width = Width1;
		height = Height1;
		visible = VisibleYes;
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

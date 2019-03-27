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
		ss << "objectType=Accessory;" << SerializeWithoutType();
		return ss.str();
	}

	std::string Accessory::SerializeWithoutType() const
	{
		stringstream ss;
		ss << LayoutItem::Serialize()
			<< ";" << LockableItem::Serialize()
			<< ";controlID=" << static_cast<int>(controlID)
			<< ";protocol=" << static_cast<int>(protocol)
			<< ";address=" << static_cast<int>(address)
			<< ";type=" << static_cast<int>(type)
			<< ";state=" << static_cast<int>(state)
			<< ";duration=" << static_cast<int>(duration)
			<< ";inverted=" << static_cast<int>(inverted);
		return ss.str();
	}

	bool Accessory::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Accessory") != 0)
		{
			return false;
		}

		return Deserialize(arguments);
	}

	bool Accessory::Deserialize(const map<string,string>& arguments)
	{
		LayoutItem::Deserialize(arguments);
		LockableItem::Deserialize(arguments);
		width = Width1;
		height = Height1;
		visible = VisibleYes;
		controlID = GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address = GetIntegerMapEntry(arguments, "address");
		type = GetIntegerMapEntry(arguments, "type");
		state = GetIntegerMapEntry(arguments, "state");
		duration = GetIntegerMapEntry(arguments, "timeout", DefaultAccessoryDuration); // FIXME: remove in later versions, is only here for conversion
		duration = GetIntegerMapEntry(arguments, "duration", DefaultAccessoryDuration);
		inverted = GetBoolMapEntry(arguments, "inverted");
		return true;
	}
} // namespace datamodel

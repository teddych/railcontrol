#include "datamodel/HardwareHandle.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string HardwareHandle::Serialize() const
	{
		stringstream ss;
		ss << "controlID=" << static_cast<int>(controlID)
			<< ";protocol=" << static_cast<int>(protocol)
			<< ";address=" << static_cast<int>(address);
		return ss.str();
	}

	bool HardwareHandle::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		return Deserialize(arguments);
	}

	bool HardwareHandle::Deserialize(const map<string,string>& arguments)
	{
		controlID = Utils::Utils::GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		protocol = static_cast<protocol_t>(Utils::Utils::GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address = Utils::Utils::GetIntegerMapEntry(arguments, "address");
		return true;
	}
} // namespace datamodel

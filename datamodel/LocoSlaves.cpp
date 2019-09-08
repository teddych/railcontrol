#include "datamodel/LocoSlaves.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;
using std::vector;

namespace datamodel
{
	std::string LocoSlaves::Serialize() const
	{
		stringstream ss;
		for (size_t i = 0; i < slaves.size(); ++i)
		{
			if (i > 0)
			{
				ss << ",";
			}
			ss << slaves[i].Serialize();
		}
		return ss.str();
	}

	bool LocoSlaves::Deserialize(const std::string& serialized)
	{
		std::vector<std::string> slaveParts;
		Utils::Utils::SplitString(serialized, ",", slaveParts);
		for (size_t i = 0; i < slaveParts.size(); ++i)
		{
			LocoMasterSlave slave(manager, masterID, slaveParts[i]);
			slaves.push_back(slave);
		}
		return true;
	}
} // namespace datamodel

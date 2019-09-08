#include <vector>

#include "datamodel/serializable.h"
#include "Utils/Utils.h"

using std::string;
using std::map;
using std::vector;

namespace datamodel
{
	void Serializable::ParseArguments(const string& serialized, map<string, string>& arguments)
	{
		vector<string> parts;
		Utils::Utils::SplitString(serialized, ";", parts);
		for (auto part : parts)
		{
			if (part.length() == 0)
			{
				continue;
			}
			vector<string> keyValue;
			Utils::Utils::SplitString(part, "=", keyValue);
			if (keyValue.size() < 2)
			{
				continue;
			}
			string value = keyValue[1];
			arguments[keyValue[0]] = value;
		}
	}
}

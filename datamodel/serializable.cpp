
#include "serializable.h"

using std::string;
using std::map;
using std::vector;

namespace datamodel {

	void Serializable::parseArguments(string serialized, map<string,string>& arguments) {
		vector<string> parts;
		str_split(serialized, ";", parts);
		for (auto part : parts) {
			if (part.length() == 0) {
				continue;
			}
			vector<string> keyValue;
			str_split(part, "=", keyValue);
			if (keyValue.size() < 2) {
				continue;
			}
			string value = keyValue[1];
			arguments[keyValue[0]] = value;
		}
	}

}

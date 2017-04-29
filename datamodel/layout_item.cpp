#include <map>
#include <sstream>
#include <string>

#include "layout_item.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	LayoutItem::LayoutItem(const objectID_t objectID, const string& name, const layoutRotation_t rotation, const layoutItemSize_t width, const layoutItemSize_t height, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) :
		Object(objectID, name),
		rotation(rotation),
		width(width),
		height(height),
		posX(posX),
		posY(posY),
		posZ(posZ) {
	}

	std::string LayoutItem::serialize() const {
		stringstream ss;
		ss << Object::serialize() << ";rotation=" << (int)rotation << ";width=" << (int)width << ";height=" << (int)height << ";posX=" << (int)posX << ";posY=" << (int)posY << ";posZ=" << (int)posZ;
		return ss.str();
	}

	bool LayoutItem::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool LayoutItem::deserialize(const map<string,string>& arguments) {
		Object::deserialize(arguments);
		if (arguments.count("rotation")) rotation = stoi(arguments.at("rotation"));
		if (arguments.count("width")) width = stoi(arguments.at("width"));
		if (arguments.count("height")) height = stoi(arguments.at("height"));
		if (arguments.count("posX")) posX = stoi(arguments.at("posX"));
		if (arguments.count("posY")) posY = stoi(arguments.at("posY"));
		if (arguments.count("posZ")) posZ = stoi(arguments.at("posZ"));
		return true;
	}

} // namespace datamodel


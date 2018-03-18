#include <map>
#include <sstream>
#include <string>

#include "layout_item.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	LayoutItem::LayoutItem(const objectID_t objectID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation) :
		Object(objectID, name),
		posX(posX),
		posY(posY),
		posZ(posZ),
		width(width),
		height(height),
		rotation(rotation) {
	}

	std::string LayoutItem::serialize() const {
		stringstream ss;
		ss << Object::serialize() << ";posX=" << (int)posX << ";posY=" << (int)posY << ";posZ=" << (int)posZ << ";width=" << (int)width << ";height=" << (int)height << ";rotation=" << (int)rotation;
		return ss.str();
	}

	bool LayoutItem::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool LayoutItem::deserialize(const map<string,string>& arguments) {
		Object::deserialize(arguments);
		if (arguments.count("posX")) posX = stoi(arguments.at("posX"));
		if (arguments.count("posY")) posY = stoi(arguments.at("posY"));
		if (arguments.count("posZ")) posZ = stoi(arguments.at("posZ"));
		if (arguments.count("width")) width = stoi(arguments.at("width"));
		if (arguments.count("height")) height = stoi(arguments.at("height"));
		if (arguments.count("rotation")) rotation = stoi(arguments.at("rotation"));
		return true;
	}

} // namespace datamodel


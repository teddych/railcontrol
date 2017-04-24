#include <map>
#include <sstream>
#include <string>

#include "layout_item.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	LayoutItem::LayoutItem(layoutRotation_t rotation, layoutItemSize_t width, layoutItemSize_t height, layoutPosition_t posX, layoutPosition_t posY, layoutPosition_t posZ) :
		rotation(rotation),
		width(width),
		height(height),
		posX(posX),
		posY(posY),
		posZ(posZ) {
	}

	std::string LayoutItem::serialize() const {
		stringstream ss;
		ss << ";rotation=" << (int)rotation << ";width=" << (int)width << ";height=" << (int)height << ";posX=" << (int)posX << ";posY=" << (int)posY << ";posZ=" << (int)posZ;
		return ss.str();
	}

	bool LayoutItem::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool LayoutItem::deserialize(const map<string,string>& arguments) {
		if (arguments.count("rotation")) rotation = stoi(arguments.at("rotation"));
		if (arguments.count("width")) width = stoi(arguments.at("width"));
		if (arguments.count("height")) height = stoi(arguments.at("height"));
		if (arguments.count("posX")) posX = stoi(arguments.at("posX"));
		if (arguments.count("posY")) posY = stoi(arguments.at("posY"));
		if (arguments.count("posZ")) posZ = stoi(arguments.at("posZ"));
		return true;
	}

} // namespace datamodel


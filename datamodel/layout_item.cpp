#include <map>
#include <sstream>
#include <string>

#include "datamodel/layout_item.h"

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

	bool LayoutItem::mapPosition(const layoutPosition_t posX,
			const layoutPosition_t posY,
			const layoutItemSize_t width,
			const layoutItemSize_t height,
			const layoutRotation_t rotation,
			layoutPosition_t& x,
			layoutPosition_t& y,
			layoutItemSize_t& w,
			layoutItemSize_t& h) {

		switch (rotation) {
			case Rotation0:
				x = posX;
				y = posY;
				w = width;
				h = height;
				return true;
			case Rotation90:
				if (posX < height) {
					return false;
				}
				x = posX + 1 - height;
				y = posY;
				w = height;
				h = width;
				return true;
			case Rotation180:
				if (posX < width || posY < height) {
					return false;
				}
				x = posX + 1 - width;
				y = posY + 1 - height;
				w = width;
				h = height;
				return true;
			case Rotation270:
				if (posY < width) {
					return false;
				}
				x = posX;
				y = posY + 1 - width;
				w = height;
				h = width;
				return true;
			default:
				return false;
		}
	}

	bool LayoutItem::checkPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) {
		if (this->posZ != posZ) {
			return true;
		}
		layoutPosition_t x;
		layoutPosition_t y;
		layoutItemSize_t w;
		layoutItemSize_t h;
		bool ret = mapPosition(this->posX, this->posY, this->width, this->height, this->rotation, x, y, w, h);
		if (ret == false) {
			return false;
		}
		for(layoutPosition_t ix = x; ix < x + w; ix++) {
			for(layoutPosition_t iy = y; iy < y + h; iy++) {
				if (ix == posX && iy == posY) {
					return false;
				}
			}
		}
		return true;
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
		if (arguments.count("rotation")) rotation = static_cast<layoutRotation_t>(stoi(arguments.at("rotation")));
		return true;
	}

} // namespace datamodel


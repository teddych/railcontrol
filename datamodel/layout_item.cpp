#include <map>
#include <sstream>
#include <string>

#include "datamodel/layout_item.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	bool LayoutItem::mapPosition(const layoutPosition_t posX,
			const layoutPosition_t posY,
			const layoutItemSize_t width,
			const layoutItemSize_t height,
			const layoutRotation_t rotation,
			layoutPosition_t& x,
			layoutPosition_t& y,
			layoutItemSize_t& w,
			layoutItemSize_t& h)
	{
		switch (rotation)
		{
			case Rotation0:
				x = posX;
				y = posY;
				w = width;
				h = height;
				return true;

			case Rotation90:
				if (posX + 1 < height)
				{
					return false;
				}
				x = posX + 1 - height;
				y = posY;
				w = height;
				h = width;
				return true;

			case Rotation180:
				if (posX + 1 < width || posY + 1 < height)
				{
					return false;
				}
				x = posX + 1 - width;
				y = posY + 1 - height;
				w = width;
				h = height;
				return true;

			case Rotation270:
				if (posY + 1 < width)
				{
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

	bool LayoutItem::checkPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ)
	{
		if (this->posZ != posZ)
		{
			return true;
		}
		layoutPosition_t x;
		layoutPosition_t y;
		layoutItemSize_t w;
		layoutItemSize_t h;
		bool ret = mapPosition(this->posX, this->posY, this->width, this->height, this->rotation, x, y, w, h);
		if (ret == false)
		{
			return false;
		}
		for(layoutPosition_t ix = x; ix < x + w; ix++)
		{
			for(layoutPosition_t iy = y; iy < y + h; iy++)
			{
				if (ix == posX && iy == posY)
				{
					return false;
				}
			}
		}
		return true;
	}

	std::string LayoutItem::serialize() const
	{
		stringstream ss;
		ss << Object::serialize()
			<< ";visible=" << static_cast<int>(visible)
			<< ";posX=" << static_cast<int>(posX)
			<< ";posY=" << static_cast<int>(posY)
			<< ";posZ=" << static_cast<int>(posZ)
			<< ";width=" << static_cast<int>(width)
			<< ";height=" << static_cast<int>(height)
			<< ";rotation=" << static_cast<int>(rotation);
		return ss.str();
	}

	bool LayoutItem::deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool LayoutItem::deserialize(const map<string,string>& arguments)
	{
		Object::deserialize(arguments);
		visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible"));
		posX = GetIntegerMapEntry(arguments, "posX", 0);
		posY = GetIntegerMapEntry(arguments, "posY", 0);
		posZ = GetIntegerMapEntry(arguments, "posZ", 0);
		width = GetIntegerMapEntry(arguments, "width", Width1);
		height = GetIntegerMapEntry(arguments, "height", Height1);
		rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		return true;
	}

	std::string LayoutItem::Rotation(layoutRotation_t rotation)
	{
		std::string rotationText;
		switch (rotation)
		{
			case Rotation90:
				rotationText = "90";
				break;

			case Rotation180:
				rotationText = "180";
				break;

			case Rotation270:
				rotationText = "270";
				break;

			case Rotation0:
			default:
				rotationText = "0";
				break;
		}
		return rotationText;
	}

} // namespace datamodel


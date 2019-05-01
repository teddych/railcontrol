#include <map>
#include <sstream>
#include <string>

#include "datamodel/layout_item.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	bool LayoutItem::MapPosition(const layoutPosition_t posX,
			const layoutPosition_t posY,
			const layoutItemSize_t width,
			const layoutItemSize_t height,
			const layoutRotation_t rotation,
			layoutPosition_t& x,
			layoutPosition_t& y,
			layoutItemSize_t& w,
			layoutItemSize_t& h)
	{
		x = posX;
		y = posY;
		switch (rotation)
		{
			case Rotation0:
			case Rotation180:
				w = width;
				h = height;
				return true;

			case Rotation90:
			case Rotation270:
				w = height;
				h = width;
				return true;

			default:
				return false;
		}
	}

	bool LayoutItem::CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ)
	{
		if (this->visible == false)
		{
			return true;
		}

		if (this->posZ != posZ)
		{
			return true;
		}

		layoutPosition_t x;
		layoutPosition_t y;
		layoutItemSize_t w;
		layoutItemSize_t h;
		bool ret = MapPosition(this->posX, this->posY, this->width, this->height, this->rotation, x, y, w, h);
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

	std::string LayoutItem::Serialize() const
	{
		stringstream ss;
		ss << Object::Serialize()
			<< ";visible=" << static_cast<int>(visible)
			<< ";posX=" << static_cast<int>(posX)
			<< ";posY=" << static_cast<int>(posY)
			<< ";posZ=" << static_cast<int>(posZ)
			<< ";width=" << static_cast<int>(width)
			<< ";height=" << static_cast<int>(height)
			<< ";rotation=" << static_cast<int>(rotation);
		return ss.str();
	}

	bool LayoutItem::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		return Deserialize(arguments);
	}

	bool LayoutItem::Deserialize(const map<string,string>& arguments)
	{
		Object::Deserialize(arguments);
		visible = static_cast<visible_t>(Utils::Utils::GetBoolMapEntry(arguments, "visible"));
		posX = Utils::Utils::GetIntegerMapEntry(arguments, "posX", 0);
		posY = Utils::Utils::GetIntegerMapEntry(arguments, "posY", 0);
		posZ = Utils::Utils::GetIntegerMapEntry(arguments, "posZ", 0);
		width = Utils::Utils::GetIntegerMapEntry(arguments, "width", Width1);
		height = Utils::Utils::GetIntegerMapEntry(arguments, "height", Height1);
		rotation = static_cast<layoutRotation_t>(Utils::Utils::GetIntegerMapEntry(arguments, "rotation", Rotation0));
		return true;
	}

	std::string LayoutItem::Rotation(layoutRotation_t rotation)
	{
		std::string rotationText;
		switch (rotation)
		{
			case Rotation90:
				return "90";

			case Rotation180:
				return "180";

			case Rotation270:
				return "270";

			case Rotation0:
			default:
				return "0";
		}
	}
} // namespace datamodel


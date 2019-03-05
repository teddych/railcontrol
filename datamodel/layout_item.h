#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel
{
	class LayoutItem : public Object
	{
		public:
			LayoutItem(const objectID_t objectID,
				const std::string& name,
				const visible_t visible,
				const layoutPosition_t posX,
				const layoutPosition_t posY,
				const layoutPosition_t posZ,
				const layoutItemSize_t width,
				const layoutItemSize_t height,
				const layoutRotation_t rotation)
			:	Object(objectID, name),
				visible(visible),
				posX(posX),
				posY(posY),
				posZ(posZ),
				width(width),
				height(height),
				rotation(rotation)
			{
			}

			LayoutItem() {};

			static bool MapPosition(const layoutPosition_t posX, const layoutPosition_t posY, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, layoutPosition_t& x, layoutPosition_t& y, layoutItemSize_t& w, layoutItemSize_t& h);

			virtual bool Position(layoutPosition_t& x, layoutPosition_t& y, layoutPosition_t& z, layoutItemSize_t& w, layoutItemSize_t& h, layoutRotation_t& r) const
			{
				z = posZ;
				r = rotation;
				return MapPosition(posX, posY, width, height, rotation, x, y, w, h);
			}


			virtual bool CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ);
			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;
			virtual std::string LayoutType() const = 0;

			virtual std::string Rotation() const { return Rotation(rotation); }
			static std::string Rotation(layoutRotation_t rotation);
			
			visible_t visible;
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t width;
			layoutItemSize_t height;
			layoutRotation_t rotation;

		protected:
			virtual bool Deserialize(const std::map<std::string,std::string>& arguments);
	};
} // namespace datamodel


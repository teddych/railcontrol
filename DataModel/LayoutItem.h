#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "DataModel/Object.h"

namespace DataModel
{
	class LayoutItem : public Object
	{
		public:
			enum layoutRotation_t : unsigned char
			{
				Rotation0 = 0,
				Rotation90,
				Rotation180,
				Rotation270
			};

			LayoutItem(const objectID_t objectID)
			:	Object(objectID),
			 	visible(VisibleYes),
			 	width(Width1),
			 	height(Height1)
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

			void SetVisible(const visible_t visible) { this->visible = visible; }
			visible_t GetVisible() const { return visible; }
			void SetPosX(const layoutPosition_t x) { this->posX = x; }
			layoutPosition_t GetPosX() const { return posX; }
			void SetPosY(const layoutPosition_t y) { this->posY = y; }
			layoutPosition_t GetPosY() const { return posY; }
			void SetPosZ(const layoutPosition_t z) { this->posZ = z; }
			layoutPosition_t GetPosZ() const { return posZ; }
			bool HasPosition(const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z) const { return posX == x && posY == y && posZ == z; }
			bool IsVisibleOnLayer(const layoutPosition_t z) const { return posZ == z && visible == VisibleYes; }
			void SetWidth(const layoutItemSize_t width) { this->width = width; }
			layoutItemSize_t GetWidth() const { return width; }
			void SetHeight(const layoutItemSize_t height) { this->height = height; }
			layoutItemSize_t GetHeight() const { return height; }
			void SetRotation(const layoutRotation_t rotation) { this->rotation = rotation; }
			layoutRotation_t GetRotation() const { return rotation; }

			virtual std::string Rotation() const { return Rotation(rotation); }
			static std::string Rotation(layoutRotation_t rotation);

		protected:
			virtual bool Deserialize(const std::map<std::string,std::string>& arguments);
			
		private:
			visible_t visible;
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t width;
			layoutItemSize_t height;
			layoutRotation_t rotation;
	};
} // namespace DataModel


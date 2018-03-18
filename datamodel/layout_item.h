#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel {

	class LayoutItem : public Object {
		public:
			LayoutItem(const objectID_t objectID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation);
			LayoutItem();

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;
			
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t width;
			layoutItemSize_t height;
			layoutRotation_t rotation;

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);
	};

	inline LayoutItem::LayoutItem() {}

} // namespace datamodel


#ifndef DATAMODEL_LAYOUT_ITEM_H
#define DATAMODEL_LAYOUT_ITEM_H

#include <map>
#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel {

	class LayoutItem : public Object {
		public:
			LayoutItem(const objectID_t objectID, const std::string& name, const layoutRotation_t rotation, const layoutItemSize_t width, const layoutItemSize_t height, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ);
			LayoutItem();

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;
			
			layoutRotation_t rotation;
			layoutItemSize_t width;
			layoutItemSize_t height;
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);
	};

	inline LayoutItem::LayoutItem() {}

} // namespace datamodel

#endif // DATAMODEL_LAYOUT_ITEM_H

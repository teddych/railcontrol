#ifndef DATAMODEL_LAYOUT_ITEM_H
#define DATAMODEL_LAYOUT_ITEM_H

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class LayoutItem : protected Serializable {
		public:
			LayoutItem(layoutRotation_t rotation, layoutItemSize_t width, layoutItemSize_t height, layoutPosition_t posX, layoutPosition_t posY, layoutPosition_t posZ);
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

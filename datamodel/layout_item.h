#ifndef DATAMODEL_LAYOUT_ITEM_H
#define DATAMODEL_LAYOUT_ITEM_H

#include "datatypes.h"

namespace datamodel {

	class LayoutItem {
		public:
			LayoutItem();
			
			layoutRotation_t rotation;
			layoutItemSize_t width;
			layoutItemSize_t height;
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
	};

	inline LayoutItem::LayoutItem() :
		rotation(0),
		width(1),
		height(1),
		posX(1),
		posY(1),
		posZ(1) {
	}

} // namespace datamodel

#endif // DATAMODEL_LAYOUT_ITEM_H

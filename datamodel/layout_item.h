#ifndef DATAMODEL_LAYOUT_ITEM_H
#define DATAMODEL_LAYOUT_ITEM_H

#include "datatypes.h"

namespace datamodel {

	class LayoutItem {
		public:
			LayoutItem(layoutRotation_t rotation, layoutItemSize_t width, layoutItemSize_t height, layoutPosition_t posX, layoutPosition_t posY, layoutPosition_t posZ);
			
			layoutRotation_t rotation;
			layoutItemSize_t width;
			layoutItemSize_t height;
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
	};

	inline LayoutItem::LayoutItem(layoutRotation_t rotation, layoutItemSize_t width, layoutItemSize_t height, layoutPosition_t posX, layoutPosition_t posY, layoutPosition_t posZ) :
		rotation(rotation),
		width(width),
		height(height),
		posX(posX),
		posY(posY),
		posZ(posZ) {
	}

} // namespace datamodel

#endif // DATAMODEL_LAYOUT_ITEM_H

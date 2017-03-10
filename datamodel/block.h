#ifndef DATAMODEL_BLOCK_H
#define DATAMODEL_BLOCK_H

#include <mutex>
#include <string>

#include "datatypes.h"
#include "layout_item.h"

namespace datamodel {

	class Block : public LayoutItem {
		public:
			Block(blockID_t blockID, std::string name, layoutItemSize_t width, layoutRotation_t rotation, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z);

			bool tryReserve(const locoID_t locoID);
			bool reserve(const locoID_t locoID);
			bool free(const locoID_t locoID);

			static void getTexts(const blockState_t state, char*& stateText);

			blockID_t blockID;
			std::string name;
		private:
			blockState_t state;
			locoID_t locoID;
			std::mutex updateMutex;
	};

} // namespace datamodel

#endif // DATAMODEL_BLOCK_H

#include "block.h"

namespace datamodel {

	Block::Block(blockID_t blockID, std::string name, layoutItemSize_t width, layoutRotation_t rotation, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(rotation, width, /*width*/ 1, x, y, z),
		blockID(blockID),
		name(name),
		state(BLOCK_STATE_FREE) {
	}

	std::string Block::serialize() const {
		return "";
	}

	bool Block::deserialize(const std::string serialized) {
		return true;
	}

	bool Block::tryReserve(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state != BLOCK_STATE_FREE) {
			return false;
		}
		state = BLOCK_STATE_RESERVED;
		this->locoID = locoID;
		return true;
	}

	bool Block::reserve(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state == BLOCK_STATE_RESERVED && this->locoID == locoID) {
			state = BLOCK_STATE_USED;
			return true;
		}
		return false;
	}

	bool Block::free(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (this->locoID == locoID || locoID == 0) {
			this->locoID = LOCO_NONE;
			state = BLOCK_STATE_FREE;
			return true;
		}
		return false;
	}

	void Block::getTexts(const blockState_t state, char*& stateText) {
		switch (state) {
			case BLOCK_STATE_FREE:
				stateText = (char*)"free";
				break;
			case BLOCK_STATE_RESERVED:
				stateText = (char*)"reserved";
				break;
			case BLOCK_STATE_USED:
				stateText = (char*)"used";
				break;
			default:
				stateText = (char*)"unknown";
		}
	}

} // namespace datamodel


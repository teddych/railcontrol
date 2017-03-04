#include "block.h"

namespace datamodel {

	Block::Block(blockID_t blockID, std::string name) :
		blockID(blockID),
		name(name),
		state(BLOCK_STATE_FREE) {
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

} // namespace datamodel


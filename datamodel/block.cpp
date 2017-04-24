
#include <map>
#include <sstream>
#include <string>

#include "block.h"

using std::map;
using std::stoi;
using std::string;

namespace datamodel {

	Block::Block(blockID_t blockID, std::string name, layoutItemSize_t width, layoutRotation_t rotation, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(rotation, width, /*width*/ 1, x, y, z),
		blockID(blockID),
		name(name),
		state(BLOCK_STATE_FREE) {
	}

	Block::Block(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Block::serialize() const {
		std::stringstream ss;
		ss << "objectType=Block;blockID=" << (int)blockID << ";name=" << name << ";state=" << (int)state << LayoutItem::serialize();
		return ss.str();
	}

	bool Block::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Block") == 0) {
			LayoutItem::deserialize(arguments);
			if (arguments.count("blockID")) blockID = stoi(arguments.at("blockID"));
			if (arguments.count("name")) name = arguments.at("name");
			if (arguments.count("state")) state = stoi(arguments.at("state"));
			return true;
		}
		return false;
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



#include <map>
#include <sstream>
#include <string>

#include "block.h"

using std::map;
using std::stoi;
using std::string;

namespace datamodel {

	Block::Block(const blockID_t blockID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const layoutItemSize_t width, const layoutRotation_t rotation) :
		LayoutItem(blockID, name, x, y, z, width, HEIGHT_1, rotation),
		state(BLOCK_STATE_FREE) /* FIXME */,
		locoID(0) /* FIXME */,
		locoDirection(false) {
	}

	Block::Block(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Block::serialize() const {
		std::stringstream ss;
		ss << "objectType=Block;" << LayoutItem::serialize() << ";state=" << (int)state << ";locoID=" << (int)locoID << ";locoDirection=" << (int)locoDirection;
		return ss.str();
	}

	bool Block::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		LayoutItem::deserialize(arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Block") == 0) {
			if (arguments.count("state")) state = stoi(arguments.at("state"));
			if (arguments.count("locoID")) locoID = stoi(arguments.at("locoID"));
			if (arguments.count("locoDirection")) locoDirection = (bool)stoi(arguments.at("locoDirection"));
			return true;
		}
		return false;
	}

	bool Block::reserve(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoID) {
			if (state == BLOCK_STATE_FREE) {
				state = BLOCK_STATE_RESERVED;
			}
			return true;
		}
		if (state != BLOCK_STATE_FREE) {
			return false;
		}
		state = BLOCK_STATE_RESERVED;
		this->locoID = locoID;
		return true;
	}

	bool Block::lock(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state != BLOCK_STATE_RESERVED) {
			return false;
		}
		if (this->locoID != locoID) {
			return false;
		}
		state = BLOCK_STATE_LOCKED;
		return true;
	}

	bool Block::release(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state == BLOCK_STATE_FREE) {
			return true;
		}
		if (this->locoID != locoID) {
			return false;
		}
		this->locoID = LOCO_NONE;
		state = BLOCK_STATE_FREE;
		return true;
	}

	bool Block::addStreet(Street* street) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		for(auto s : streets) {
			if (s == street) return false;
		}
		streets.push_back(street);
		return true;
	}

	bool Block::removeStreet(Street* street) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		/* FIXME */
		return false;
	}

	bool Block::getValidStreets(std::vector<Street*>& validStreets) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		for(auto street : streets) {
			if (street->fromBlockDirection(objectID, locoDirection)) {
				validStreets.push_back(street);
			}
		}
		return true;
	}

	void Block::getTexts(const blockState_t state, string& stateText) {
		switch (state) {
			case BLOCK_STATE_FREE:
				stateText.assign("free");
				break;
			case BLOCK_STATE_RESERVED:
				stateText.assign("reserved");
				break;
			case BLOCK_STATE_LOCKED:
				stateText.assign("locked");
				break;
			default:
				stateText.assign("unknown");
		}
	}

} // namespace datamodel


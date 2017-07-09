
#include <map>
#include <sstream>

#include "street.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Street::Street(Manager* manager, const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const direction_t fromDirection, const blockID_t toBlock, const direction_t toDirection) :
		Object(streetID, name),
		manager(manager),
		state(STREET_STATE_FREE),
		fromBlock(fromBlock),
		fromDirection(fromDirection),
		toBlock(toBlock),
		toDirection(toDirection),
		locoID(LOCO_NONE) {
		Block* block = manager->getBlock(fromBlock);
		if (!block) return;
		block->addStreet(this);
	}

	Street::Street(Manager* manager, const std::string& serialized) :
		manager(manager),
		locoID(LOCO_NONE) {
		deserialize(serialized);
		Block* block = manager->getBlock(fromBlock);
		if (!block) return;
		block->addStreet(this);
	}

	std::string Street::serialize() const {
		stringstream ss;
		ss << "objectType=Street;" << Object::serialize() << ";state=" << (int)state << ";fromBlock=" << (int)fromBlock << ";fromDirection=" << (int)fromDirection << ";toBlock=" << (int)toBlock << ";toDirection=" << (int)toDirection;
		return ss.str();
	}

	bool Street::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Street") == 0) {
			Object::deserialize(arguments);
			if (arguments.count("state")) state = stoi(arguments.at("state"));
			if (arguments.count("fromBlock")) fromBlock = stoi(arguments.at("fromBlock"));
			if (arguments.count("fromDirection")) fromDirection = (bool)stoi(arguments.at("fromDirection"));
			if (arguments.count("toBlock")) toBlock = stoi(arguments.at("toBlock"));
			if (arguments.count("toDirection")) toDirection = (bool)stoi(arguments.at("toDirection"));
			return true;
		}
		return false;
	}

	bool Street::reserve(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoID) return true;
		if (state != STREET_STATE_FREE) return false;
		Block* block = manager->getBlock(toBlock);
		if (!block) return false;
		if (!block->reserve(locoID)) return false;
		state = STREET_STATE_RESERVED;
		this->locoID = locoID;
		return true;
	}

	bool Street::lock(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state != STREET_STATE_RESERVED) return false;
		if (this->locoID != locoID) return false;
		Block* block = manager->getBlock(toBlock);
		if (!block) return false;
		if (!block->lock(locoID)) return false;
		state = STREET_STATE_LOCKED;
		return true;
	}

	bool Street::release(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state == STREET_STATE_FREE) return true;
		if (this->locoID != locoID) return false;
		Block* block = manager->getBlock(toBlock);
		block->release(locoID);
		this->locoID = LOCO_NONE;
		state = STREET_STATE_FREE;
		return true;
	}

} // namespace datamodel



#include <map>
#include <sstream>

#include "street.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Street::Street(Manager* manager, const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const direction_t fromDirection, const blockID_t toBlock, const direction_t toDirection, const feedbackID_t feedbackIDStop) :
		Object(streetID, name),
		fromBlock(fromBlock),
		fromDirection(fromDirection),
		toBlock(toBlock),
		toDirection(toDirection),
		feedbackIDStop(feedbackIDStop),
		manager(manager),
		lockState(LockStateFree),
		locoID(LocoNone) {
		Block* block = manager->getBlock(fromBlock);
		if (!block) return;
		block->addStreet(this);
	}

	Street::Street(Manager* manager, const std::string& serialized) :
		manager(manager),
		locoID(LocoNone) {
		deserialize(serialized);
		Block* block = manager->getBlock(fromBlock);
		if (!block) return;
		block->addStreet(this);
	}

	std::string Street::serialize() const {
		stringstream ss;
		ss << "objectType=Street;" << Object::serialize() << ";lockState=" << (int)lockState << ";fromBlock=" << (int)fromBlock << ";fromDirection=" << (int)fromDirection << ";toBlock=" << (int)toBlock << ";toDirection=" << (int)toDirection << ";feedbackIDStop=" << (int)feedbackIDStop;
		return ss.str();
	}

	bool Street::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Street") == 0) {
			Object::deserialize(arguments);
			if (arguments.count("lockState")) lockState = stoi(arguments.at("lockState"));
			if (arguments.count("fromBlock")) fromBlock = stoi(arguments.at("fromBlock"));
			if (arguments.count("fromDirection")) fromDirection = (bool)stoi(arguments.at("fromDirection"));
			if (arguments.count("toBlock")) toBlock = stoi(arguments.at("toBlock"));
			if (arguments.count("toDirection")) toDirection = (bool)stoi(arguments.at("toDirection"));
			if (arguments.count("feedbackIDStop")) feedbackIDStop = stoi(arguments.at("feedbackIDStop"));
			return true;
		}
		return false;
	}

	bool Street::reserve(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoID) {
			return true;
		}
		if (lockState != LockStateFree) {
			return false;
		}
		Block* block = manager->getBlock(toBlock);
		if (!block) {
			return false;
		}
		if (!block->reserve(locoID)) {
			return false;
		}
		lockState = LockStateReserved;
		this->locoID = locoID;
		return true;
	}

	bool Street::lock(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState != LockStateReserved) {
			return false;
		}
		if (this->locoID != locoID) {
			return false;
		}
		Block* block = manager->getBlock(toBlock);
		if (!block) {
			return false;
		}
		if (!block->lock(locoID)) {
			return false;
		}
		lockState = LockStateHardLocked;
		Feedback* feedback = manager->getFeedback(feedbackIDStop);
		feedback->setLoco(locoID);
		return true;
	}

	bool Street::release(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState == LockStateFree) {
			return true;
		}
		if (this->locoID != locoID) {
			return false;
		}
		Block* block = manager->getBlock(fromBlock);
		block->release(locoID);
		this->locoID = LocoNone;
		lockState = LockStateFree;
		Feedback* feedback = manager->getFeedback(feedbackIDStop);
		feedback->setLoco(LocoNone);
		return true;
	}

} // namespace datamodel


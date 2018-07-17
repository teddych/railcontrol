#include <map>
#include <sstream>

#include "switch.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Switch::Switch(switchID_t switchID,
		std::string name,
		layoutPosition_t x,
		layoutPosition_t y,
		layoutPosition_t z,
		layoutRotation_t rotation,
		controlID_t controlID,
		protocol_t protocol,
		address_t address,
		switchType_t type,
		switchState_t state,
		switchTimeout_t timeout)
	:	Accessory(switchID, name, x, y, z, rotation, controlID, protocol, address, type, state << 1, timeout),
		lockState(LockStateFree),
		locoIDHardLock(LocoNone) {
	}

	Switch::Switch(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Switch::serialize() const {
		stringstream ss;
		ss << "objectType=Switch;" << Accessory::serializeWithoutType() << ";lockState=" << static_cast<int>(lockState) << ";locoIDHardLock" << static_cast<int>(locoIDHardLock); // FIXME: locoIDSoftLock is missing
		return ss.str();
	}

	bool Switch::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Switch") == 0) {
			if (arguments.count("lockState")) lockState = static_cast<lockState_t>(stoi(arguments.at("lockState")));
			if (arguments.count("locoIDHardLock")) locoIDHardLock = stoi(arguments.at("locoIDHardLock"));
			// FIXME: if (arguments.count("locoIDSoftLock")) locoID = stoi(arguments.at("locoIDSoftLock"));
			Accessory::deserialize(arguments);
			return true;
		}
		return false;
	}

	bool Switch::reserve(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoIDHardLock) {
			if (lockState == LockStateFree) {
				lockState = LockStateReserved;
			}
			return true;
		}
		if (lockState != LockStateFree) {
			return false;
		}
		lockState = LockStateReserved;
		this->locoIDHardLock = locoID;
		return true;
	}

	bool Switch::hardLock(const locoID_t locoID, const switchState_t switchState) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState != LockStateReserved) {
			return false;
		}
		if (this->locoIDHardLock != locoID) {
			return false;
		}
		lockState = LockStateHardLocked;
		state = switchState;
		return true;
	}

	bool Switch::softLock(const locoID_t locoID, const switchState_t switchState) {
		// FIXME: not implemented
		return false;
	}

	bool Switch::release(const locoID_t locoID) {
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState == LockStateFree) {
			return true;
		}
		if (this->locoIDHardLock != locoID) {
			return false;
		}
		this->locoIDHardLock = LocoNone;
		lockState = LockStateFree;
		return true;
	}
} // namespace datamodel


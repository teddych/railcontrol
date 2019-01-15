#include <map>
#include <sstream>
#include <string>

#include "datamodel/relation.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Relation::serialize() const {
		stringstream ss;
		ss << "relationID=" << static_cast<int>(relationID)
			<< ";name=" << name
			<< ";objectType1=" << static_cast<int>(objectType1)
			<< ";objectID1=" << objectID1
			<< ";objectType2=" << static_cast<int>(objectType2)
			<< ";objectID2=" << objectID2
			<< ";switchState=" << static_cast<int>(switchState)
			<< ";lockState=" << static_cast<int>(lockState);
		return ss.str();
	}

	bool Relation::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool Relation::deserialize(const map<string,string>& arguments) {
		relationID = GetIntegerMapEntry(arguments, "relationID", RelationNone);
		name = GetStringMapEntry(arguments, "name");
		objectType1 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType1"));
		objectID1 = GetIntegerMapEntry(arguments, "objectID1");
		objectType2 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType2"));
		objectID2 = GetIntegerMapEntry(arguments, "objectID2");
		switchState = GetIntegerMapEntry(arguments, "switchState");
		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		return true;
	}

	bool Relation::execute(const locoID_t locoID) {
		if (objectType2 != ObjectTypeSwitch) {
			return false;
		}
		Switch* mySwitch = manager->getSwitch(objectID2);
		if (mySwitch == nullptr) {
			return false;
		}
		if (!mySwitch->reserve(locoID)) {
			return false;
		}
		if (lockState == LockStateHardLocked) {
			mySwitch->hardLock(locoID, switchState);
		}
		else {
			mySwitch->softLock(locoID, switchState);
		}

		manager->handleSwitch(ControlTypeAutomode, objectID2, switchState);
		return true;
	}

	bool Relation::release(const locoID_t locoID) {
		if (objectType2 != ObjectTypeSwitch) {
			return false;
		}
		Switch* mySwitch = manager->getSwitch(objectID2);
		if (mySwitch == nullptr) {
			return false;
		}
		return mySwitch->release(locoID);
	}

} // namespace datamodel


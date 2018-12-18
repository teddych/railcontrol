#include <map>
#include <sstream>

#include "datamodel/switch.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Switch::serialize() const
	{
		stringstream ss;
		ss << "objectType=Switch;"
			<< Accessory::serializeWithoutType()
			<< ";lockState=" << static_cast<int>(lockState)
			<< ";locoIDHardLock" << static_cast<int>(locoIDHardLock); // FIXME: locoIDSoftLock is missing
		return ss.str();
	}

	bool Switch::deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		string objectType = GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Switch") != 0)
		{
			return false;
		}

		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		locoIDHardLock = GetIntegerMapEntry(arguments, "locoIDHardLock", LocoNone);
		// FIXME: locoIDSoftLock
		Accessory::deserialize(arguments);
		return true;
	}

	bool Switch::reserve(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoIDHardLock)
		{
			if (lockState == LockStateFree)
			{
				lockState = LockStateReserved;
			}
			return true;
		}
		if (lockState != LockStateFree)
		{
			return false;
		}
		lockState = LockStateReserved;
		this->locoIDHardLock = locoID;
		return true;
	}

	bool Switch::hardLock(const locoID_t locoID, const switchState_t switchState)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState != LockStateReserved)
		{
			return false;
		}
		if (this->locoIDHardLock != locoID)
		{
			return false;
		}
		lockState = LockStateHardLocked;
		state = switchState;
		return true;
	}

	bool Switch::softLock(const locoID_t locoID, const switchState_t switchState)
	{
		// FIXME: not implemented
		return false;
	}

	bool Switch::release(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState == LockStateFree)
		{
			return true;
		}
		if (this->locoIDHardLock != locoID)
		{
			return false;
		}
		this->locoIDHardLock = LocoNone;
		lockState = LockStateFree;
		return true;
	}
} // namespace datamodel


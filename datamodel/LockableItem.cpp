#include "datamodel/LockableItem.h"
#include "util.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	string LockableItem::Serialize() const
	{
		return "lockState=" + std::to_string(lockState) + ";locoID=" + std::to_string(locoID);

	}

	bool LockableItem::Deserialize(const map<string, string> arguments)
	{
		locoID = GetIntegerMapEntry(arguments, "locoID", LocoNone);
		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		return true;
	}

	bool LockableItem::Reserve(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(lockMutex);
		if (locoID == this->locoID)
		{
			if (lockState == LockStateFree)
			{
				lockState = LockStateReserved;
			}
			return true;
		}
		if (locoID != LocoNone || lockState != LockStateFree)
		{
			return false;
		}
		lockState = LockStateReserved;
		this->locoID = locoID;
		return true;
	}

	bool LockableItem::Lock(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(lockMutex);
		if (lockState != LockStateReserved || this->locoID != locoID)
		{
			return false;
		}
		lockState = LockStateHardLocked;
		return true;
	}

	bool LockableItem::Release(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(lockMutex);
		if (this->locoID != locoID && locoID != LocoNone)
		{
			return false;
		}
		this->locoID = LocoNone;
		lockState = LockStateFree;
		return true;
	}

} // namespace datamodel


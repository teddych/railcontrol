
#include <map>
#include <sstream>

#include "datamodel/street.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Street::Street(Manager* manager,
		const streetID_t streetID,
		const std::string& name,
		const trackID_t fromTrack,
		const direction_t fromDirection,
		const trackID_t toTrack,
		const direction_t toDirection,
		const feedbackID_t feedbackIDStop)
	:	Object(streetID, name),
		fromTrack(fromTrack),
		fromDirection(fromDirection),
		toTrack(toTrack),
		toDirection(toDirection),
		feedbackIDStop(feedbackIDStop),
		manager(manager),
		lockState(LockStateFree),
		locoID(LocoNone)
	{
		Track* track = manager->getTrack(fromTrack);
		if (!track) return;
		track->addStreet(this);
	}

	Street::Street(Manager* manager, const std::string& serialized)
	:	manager(manager),
		locoID(LocoNone)
	{
		deserialize(serialized);
		Track* track = manager->getTrack(fromTrack);
		if (!track)
		{
			return;
		}
		track->addStreet(this);
	}

	std::string Street::serialize() const
	{
		stringstream ss;
		ss << "objectType=Street;" << Object::serialize() << ";lockState=" << static_cast<int>(lockState) << ";fromTrack=" << static_cast<int>(fromTrack) << ";fromDirection=" << static_cast<int>(fromDirection) << ";toTrack=" << (int)toTrack << ";toDirection=" << (int)toDirection << ";feedbackIDStop=" << (int)feedbackIDStop;
		return ss.str();
	}

	bool Street::deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Street") == 0)
		{
			Object::deserialize(arguments);
			lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
			fromTrack = GetIntegerMapEntry(arguments, "fromTrack", TrackNone);
			fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromDirection", DirectionLeft));
			toTrack = GetIntegerMapEntry(arguments, "lockState", TrackNone);
			toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "toDirection", DirectionLeft));
			feedbackIDStop = GetIntegerMapEntry(arguments, "feedbackIDStop", FeedbackNone);
			return true;
		}
		return false;
	}

	bool Street::reserve(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoID)
		{
			return true;
		}
		if (lockState != LockStateFree)
		{
			return false;
		}
		Track* track = manager->getTrack(toTrack);
		if (!track)
		{
			return false;
		}
		if (!track->reserve(locoID))
		{
			return false;
		}
		lockState = LockStateReserved;
		this->locoID = locoID;
		return true;
	}

	bool Street::lock(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState != LockStateReserved)
		{
			return false;
		}
		if (this->locoID != locoID)
		{
			return false;
		}
		Track* track = manager->getTrack(toTrack);
		if (!track)
		{
			return false;
		}
		if (!track->lock(locoID))
		{
			return false;
		}
		lockState = LockStateHardLocked;
		Feedback* feedback = manager->getFeedback(feedbackIDStop);
		feedback->setLoco(locoID);
		return true;
	}

	bool Street::release(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState == LockStateFree)
		{
			return true;
		}
		if (this->locoID != locoID)
		{
			return false;
		}
		Track* track = manager->getTrack(fromTrack);
		track->release(locoID);
		this->locoID = LocoNone;
		lockState = LockStateFree;
		Feedback* feedback = manager->getFeedback(feedbackIDStop);
		feedback->setLoco(LocoNone);
		return true;
	}

} // namespace datamodel


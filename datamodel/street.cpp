
#include <map>
#include <sstream>

#include "datamodel/street.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	Street::Street(Manager* manager,
		const streetID_t streetID,
		const std::string& name,
		const delay_t delay,
		const std::vector<datamodel::Relation*>& relations,
		const visible_t visible,
		const layoutPosition_t posX,
		const layoutPosition_t posY,
		const layoutPosition_t posZ,
		const automode_t automode,
		const trackID_t fromTrack,
		const direction_t fromDirection,
		const trackID_t toTrack,
		const direction_t toDirection,
		const feedbackID_t feedbackIDStop)
	:	LayoutItem(streetID, name, visible, posX, posY, posZ, Width1, Height1, Rotation0),
	 	automode(automode),
		fromTrack(fromTrack),
		fromDirection(fromDirection),
		toTrack(toTrack),
		toDirection(toDirection),
		feedbackIDStop(feedbackIDStop),
		manager(manager),
		relations(relations),
		lockState(LockStateFree),
		locoID(LocoNone)
	{
		Track* track = manager->getTrack(fromTrack);
		if (track == nullptr)
		{
			return;
		}
		track->addStreet(this);
	}

	Street::Street(Manager* manager, const std::string& serialized)
	:	manager(manager),
		locoID(LocoNone)
	{
		Deserialize(serialized);
		Track* track = manager->getTrack(fromTrack);
		if (track == nullptr)
		{
			return;
		}
		track->addStreet(this);
	}

	std::string Street::Serialize() const
	{
		stringstream ss;
		ss << "objectType=Street;"
			<< LayoutItem::Serialize()
			<< ";delay=" << static_cast<int>(delay)
			<< ";lockState=" << static_cast<int>(lockState)
			<< ";automode=" << static_cast<int>(automode)
			<< ";fromTrack=" << static_cast<int>(fromTrack)
			<< ";fromDirection=" << static_cast<int>(fromDirection)
			<< ";toTrack=" << static_cast<int>(toTrack)
			<< ";toDirection=" << static_cast<int>(toDirection)
			<< ";feedbackIDStop=" << static_cast<int>(feedbackIDStop);
		return ss.str();
	}

	bool Street::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Street") == 0)
		{
			LayoutItem::Deserialize(arguments);
			delay = static_cast<delay_t>(GetIntegerMapEntry(arguments, "delay", 250));
			lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
			automode = static_cast<automode_t>(GetBoolMapEntry(arguments, "automode", AutomodeNo));
			fromTrack = GetIntegerMapEntry(arguments, "fromTrack", TrackNone);
			fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromDirection", DirectionLeft));
			toTrack = GetIntegerMapEntry(arguments, "lockState", TrackNone);
			toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "toDirection", DirectionLeft));
			feedbackIDStop = GetIntegerMapEntry(arguments, "feedbackIDStop", FeedbackNone);
			return true;
		}
		return false;
	}

	void Street::DeleteRelations()
	{
		while (!relations.empty())
		{
			delete relations.back();
			relations.pop_back();
		}
	}

	bool Street::AssignRelations(const std::vector<datamodel::Relation*>& newRelations)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState != LockStateFree)
		{
			return false;
		}
		DeleteRelations();
		relations = newRelations;
		return true;
	}

	bool Street::Execute()
	{
		bool ret = true;
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto relation : relations)
		{
			switch (relation->ObjectType2())
			{
				case ObjectTypeSwitch:
					manager->SwitchState(ControlTypeInternal, relation->ObjectID2(), relation->AccessoryState());
					break;

				default:
					ret = false;
					break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
		return ret;
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
		feedback->SetLoco(locoID);
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
		feedback->SetLoco(LocoNone);
		return true;
	}

} // namespace datamodel


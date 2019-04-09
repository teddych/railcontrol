
#include <map>
#include <sstream>

#include "datamodel/loco.h"
#include "datamodel/relation.h"
#include "datamodel/street.h"
#include "manager.h"
#include "util.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	Street::Street(Manager* manager,
		const streetID_t streetID,
		const std::string& name,
		const delay_t delay,
		const commuterType_t commuter,
		const length_t minTrainLength,
		const length_t maxTrainLength,
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
		const feedbackID_t feedbackIdReduced,
		const feedbackID_t feedbackIdCreep,
		const feedbackID_t feedbackIdStop,
		const feedbackID_t feedbackIdOver)
	:	LayoutItem(streetID, name, visible, posX, posY, posZ, Width1, Height1, Rotation0),
	 	LockableItem(),
	 	delay(delay),
	 	commuter(commuter),
		minTrainLength(minTrainLength),
		maxTrainLength(maxTrainLength),
	 	automode(automode),
		fromTrack(fromTrack),
		fromDirection(fromDirection),
		toTrack(toTrack),
		toDirection(toDirection),
		feedbackIdReduced(feedbackIdReduced),
		feedbackIdCreep(feedbackIdCreep),
		feedbackIdStop(feedbackIdStop),
		feedbackIdOver(feedbackIdOver),
		manager(manager),
		relations(relations),
		lastUsed(0),
		counter(0)
	{
		Track* track = manager->GetTrack(fromTrack);
		if (track == nullptr)
		{
			return;
		}
		track->AddStreet(this);
	}

	Street::Street(Manager* manager, const std::string& serialized)
	:	LockableItem(),
	 	manager(manager)
	{
		Deserialize(serialized);
		Track* track = manager->GetTrack(fromTrack);
		if (track == nullptr)
		{
			return;
		}
		track->AddStreet(this);
	}

	std::string Street::Serialize() const
	{
		stringstream ss;
		ss << "objectType=Street;"
			<< LayoutItem::Serialize()
			<< ";" << LockableItem::Serialize()
			<< ";delay=" << static_cast<int>(delay)
			<< ";commuter=" << static_cast<int>(commuter)
			<< ";mintrainlength=" << static_cast<int>(minTrainLength)
			<< ";maxtrainlength=" << static_cast<int>(maxTrainLength)
			<< ";lastused=" << lastUsed
			<< ";counter=" << counter
			<< ";automode=" << static_cast<int>(automode)
			<< ";fromTrack=" << static_cast<int>(fromTrack)
			<< ";fromDirection=" << static_cast<int>(fromDirection)
			<< ";toTrack=" << static_cast<int>(toTrack)
			<< ";toDirection=" << static_cast<int>(toDirection)
			<< ";feedbackIdReduced=" << static_cast<int>(feedbackIdReduced)
			<< ";feedbackIdCreep=" << static_cast<int>(feedbackIdCreep)
			<< ";feedbackIdStop=" << static_cast<int>(feedbackIdStop)
			<< ";feedbackIdOver=" << static_cast<int>(feedbackIdOver);
		return ss.str();
	}

	bool Street::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Street") != 0)
		{
			return false;
		}

		LayoutItem::Deserialize(arguments);
		LockableItem::Deserialize(arguments);

		delay = static_cast<delay_t>(GetIntegerMapEntry(arguments, "delay", DefaultDelay));
		commuter = static_cast<commuterType_t>(GetIntegerMapEntry(arguments, "commuter", CommuterTypeBoth));
		minTrainLength = static_cast<length_t>(GetIntegerMapEntry(arguments, "mintrainlength", 0));
		maxTrainLength = static_cast<length_t>(GetIntegerMapEntry(arguments, "maxtrainlength", 0));
		lastUsed = GetIntegerMapEntry(arguments, "lastused", 0);
		counter = GetIntegerMapEntry(arguments, "counter", 0);
		automode = static_cast<automode_t>(GetBoolMapEntry(arguments, "automode", AutomodeNo));
		fromTrack = GetIntegerMapEntry(arguments, "fromTrack", TrackNone);
		fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromDirection", DirectionRight));
		toTrack = GetIntegerMapEntry(arguments, "toTrack", TrackNone);
		toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "toDirection", DirectionLeft));
		feedbackIdReduced = GetIntegerMapEntry(arguments, "feedbackIdReduced", FeedbackNone);
		feedbackIdCreep = GetIntegerMapEntry(arguments, "feedbackIdCreep", FeedbackNone);
		feedbackIdStop = GetIntegerMapEntry(arguments, "feedbackIdStop", FeedbackNone);
		feedbackIdOver = GetIntegerMapEntry(arguments, "feedbackIdOver", FeedbackNone);
		return true;
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
		if (GetLockState() != LockStateFree)
		{
			return false;
		}
		DeleteRelations();
		relations = newRelations;
		return true;
	}

	bool Street::FromTrackDirection(const trackID_t trackID, const direction_t trackDirection, const bool locoCommuter)
	{
		if (automode == false)
		{
			return false;
		}

		if (fromTrack != trackID)
		{
			return false;
		}

		if (commuter != locoCommuter && commuter != CommuterTypeBoth)
		{
			return false;
		}

		if (locoCommuter == true)
		{
			return true;
		}

		return fromDirection == trackDirection;
	}


	bool Street::Execute()
	{
		bool ret = true;
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto relation : relations)
		{
			ret &= relation->Execute();
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
		lastUsed = time(nullptr);
		++counter;
		return ret;
	}

	bool Street::Reserve(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		bool ret = LockableItem::Reserve(locoID);
		if (ret == false)
		{
			return false;
		}

		Track* track = manager->GetTrack(toTrack);
		if (track == nullptr || track->Reserve(locoID) == false)
		{
			ReleaseInternal(locoID);
			return false;
		}

		ret = true;
		for (auto relation : relations)
		{
			ret &= relation->Reserve(locoID);
		}
		if (ret == false)
		{
			ReleaseInternal(locoID);
			return false;
		}
		return true;
	}

	bool Street::Lock(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		bool ret = LockableItem::Lock(locoID);
		if (ret == false)
		{
			return false;
		}

		Track* track = manager->GetTrack(toTrack);
		if (track == nullptr || track->Lock(locoID) == false)
		{
			ReleaseInternal(locoID);
			return false;
		}

		ret = true;
		for (auto relation : relations)
		{
			ret &= relation->Lock(locoID);
		}
		if (ret == false)
		{
			ReleaseInternal(locoID);
			return false;
		}

		return true;
	}

	bool Street::Release(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		return ReleaseInternal(locoID);
	}

	bool Street::ReleaseInternal(const locoID_t locoID)
	{
		for (auto relation : relations)
		{
			relation->Release(locoID);
		}
		return LockableItem::Release(locoID);
	}
} // namespace datamodel


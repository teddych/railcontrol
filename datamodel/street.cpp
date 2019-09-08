
#include <map>
#include <sstream>

#include "datamodel/Loco.h"
#include "datamodel/relation.h"
#include "datamodel/street.h"
#include "manager.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
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
			<< ";feedbackIdOver=" << static_cast<int>(feedbackIdOver)
			<< ";waitafterrelease=" << static_cast<int>(waitAfterRelease);
		return ss.str();
	}

	bool Street::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = Utils::Utils::GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Street") != 0)
		{
			return false;
		}

		LayoutItem::Deserialize(arguments);
		LockableItem::Deserialize(arguments);

		delay = static_cast<delay_t>(Utils::Utils::GetIntegerMapEntry(arguments, "delay", DefaultDelay));
		commuter = static_cast<commuterType_t>(Utils::Utils::GetIntegerMapEntry(arguments, "commuter", CommuterTypeBoth));
		minTrainLength = static_cast<length_t>(Utils::Utils::GetIntegerMapEntry(arguments, "mintrainlength", 0));
		maxTrainLength = static_cast<length_t>(Utils::Utils::GetIntegerMapEntry(arguments, "maxtrainlength", 0));
		lastUsed = Utils::Utils::GetIntegerMapEntry(arguments, "lastused", 0);
		counter = Utils::Utils::GetIntegerMapEntry(arguments, "counter", 0);
		automode = static_cast<automode_t>(Utils::Utils::GetBoolMapEntry(arguments, "automode", AutomodeNo));
		fromTrack = Utils::Utils::GetIntegerMapEntry(arguments, "fromTrack", TrackNone);
		fromDirection = static_cast<direction_t>(Utils::Utils::GetBoolMapEntry(arguments, "fromDirection", DirectionRight));
		toTrack = Utils::Utils::GetIntegerMapEntry(arguments, "toTrack", TrackNone);
		toDirection = static_cast<direction_t>(Utils::Utils::GetBoolMapEntry(arguments, "toDirection", DirectionLeft));
		feedbackIdReduced = Utils::Utils::GetIntegerMapEntry(arguments, "feedbackIdReduced", FeedbackNone);
		feedbackIdCreep = Utils::Utils::GetIntegerMapEntry(arguments, "feedbackIdCreep", FeedbackNone);
		feedbackIdStop = Utils::Utils::GetIntegerMapEntry(arguments, "feedbackIdStop", FeedbackNone);
		feedbackIdOver = Utils::Utils::GetIntegerMapEntry(arguments, "feedbackIdOver", FeedbackNone);
		waitAfterRelease = Utils::Utils::GetIntegerMapEntry(arguments, "waitafterrelease", 0);
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

	bool Street::FromTrackDirection(const trackID_t trackID, const direction_t trackDirection, const Loco* loco, const bool allowLocoTurn)
	{
		if (automode == false)
		{
			return false;
		}

		if (fromTrack != trackID)
		{
			return false;
		}

		const length_t locoLength = loco->GetLength();
		if (locoLength < minTrainLength)
		{
			return false;
		}
		if (maxTrainLength > 0 && locoLength > maxTrainLength)
		{
			return false;
		}

		const bool locoCommuter = loco->GetCommuter();
		if (commuter != locoCommuter && commuter != CommuterTypeBoth)
		{
			return false;
		}

		if (allowLocoTurn == true && locoCommuter == true)
		{
			return true;
		}

		return fromDirection == trackDirection;
	}


	bool Street::Execute()
	{
		if (manager->Booster() == BoosterStop)
		{
			return false;
		}

		bool ret = true;
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto relation : relations)
		{
			ret &= relation->Execute(delay);
		}
		lastUsed = time(nullptr);
		++counter;
		return ret;
	}

	bool Street::Reserve(const locoID_t locoID)
	{
		if (manager->Booster() == BoosterStop)
		{
			return false;
		}

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
		if (manager->Booster() == BoosterStop)
		{
			return false;
		}

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
			ReleaseInternalWithToTrack(locoID);
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

	void Street::ReleaseInternalWithToTrack(const locoID_t locoID)
	{
		Track* track = manager->GetTrack(toTrack);
		if (track != nullptr)
		{
			track->Release(locoID);
		}
		ReleaseInternal(locoID);
	}
} // namespace datamodel


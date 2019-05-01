#include <algorithm>
#include <map>
#include <sstream>
#include <string>

#include "datamodel/feedback.h"
#include "datamodel/track.h"
#include "manager.h"
#include "Utils/Utils.h"

using std::map;
using std::string;
using std::vector;

namespace datamodel
{
	std::string Track::Serialize() const
	{
		std::string feedbackString;
		for (auto feedback : feedbacks)
		{
			if (feedbackString.size() > 0)
			{
				feedbackString += ",";
			}
			feedbackString += std::to_string(feedback);
		}
		std::stringstream ss;
		ss << "objectType=Track;"
			<< LayoutItem::Serialize()
			<< ";" << LockableItem::Serialize()
			<< ";type=" << static_cast<int>(type)
			<< ";feedbacks=" << feedbackString
			<< ";selectstreetapproach=" << static_cast<int>(selectStreetApproach)
			<< ";state=" << static_cast<int>(state)
			<< ";statedelayed=" << static_cast<int>(stateDelayed)
			<< ";locoDirection=" << static_cast<int>(locoDirection)
			<< ";blocked=" << static_cast<int>(blocked)
			<< ";locodelayed=" << static_cast<int>(locoIdDelayed);
		return ss.str();
	}

	bool Track::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		ParseArguments(serialized, arguments);
		LayoutItem::Deserialize(arguments);
		LockableItem::Deserialize(arguments);
		SetWidth(Width1);
		SetVisible(VisibleYes);
		string objectType = Utils::Utils::GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Track") != 0)
		{
			return false;
		}
		type = static_cast<trackType_t>(Utils::Utils::GetBoolMapEntry(arguments, "type", TrackTypeStraight));
		string feedbackStrings = Utils::Utils::GetStringMapEntry(arguments, "feedbacks");
		vector<string> feedbackStringVector;
		Utils::Utils::SplitString(feedbackStrings, ",", feedbackStringVector);
		for (auto feedbackString : feedbackStringVector)
		{
			feedbacks.push_back(Utils::Utils::StringToInteger(feedbackString));
		}
		selectStreetApproach = static_cast<selectStreetApproach_t>(Utils::Utils::GetIntegerMapEntry(arguments, "selectstreetapproach", SelectStreetSystemDefault));
		state = static_cast<feedbackState_t>(Utils::Utils::GetBoolMapEntry(arguments, "state", FeedbackStateFree));
		stateDelayed = static_cast<feedbackState_t>(Utils::Utils::GetBoolMapEntry(arguments, "statedelayed", state));
		locoDirection = static_cast<direction_t>(Utils::Utils::GetBoolMapEntry(arguments, "locoDirection", DirectionRight));
		blocked = Utils::Utils::GetBoolMapEntry(arguments, "blocked", false);
		locoIdDelayed = static_cast<locoID_t>(Utils::Utils::GetIntegerMapEntry(arguments, "locodelayed", GetLoco()));
		return true;
	}

	bool Track::Reserve(const locoID_t locoID)
	{
		if (this->locoIdDelayed != LocoNone)
		{
			return false;
		}
		if (blocked == true)
		{
			return false;
		}
		if (state != FeedbackStateFree)
		{
			return false;
		}
		return ReserveForce(locoID);
	}

	bool Track::ReserveForce(const locoID_t locoID)
	{
		bool ret = LockableItem::Reserve(locoID);
		if (ret == false)
		{
			return false;
		}
		this->locoIdDelayed = locoID;
		return true;
	}

	bool Track::Lock(const locoID_t locoID)
	{
		bool ret = LockableItem::Lock(locoID);
		manager->TrackPublishState(this);
		return ret;
	}

	bool Track::Release(const locoID_t locoID)
	{
		bool ret = LockableItem::Release(locoID);
		if (ret == false)
		{
			return false;
		}
		if (state != FeedbackStateFree)
		{
			return true;
		}
		this->locoIdDelayed = LocoNone;
		this->stateDelayed = FeedbackStateFree;
		manager->TrackPublishState(this);
		return true;
	}

	bool Track::SetFeedbackState(const feedbackID_t feedbackID, const feedbackState_t state)
	{
		feedbackState_t oldState = this->state;
		bool ret = FeedbackStateInternal(feedbackID, state);
		if (ret == false)
		{
			return false;
		}
		if (oldState == state)
		{
			return true;
		}
		manager->TrackPublishState(this);
		return true;
	}

	bool Track::FeedbackStateInternal(const feedbackID_t feedbackID, const feedbackState_t state)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state == FeedbackStateOccupied)
		{
			Loco* loco = manager->GetLoco(GetLocoDelayed());
			if (loco == nullptr)
			{
				manager->Booster(ControlTypeInternal, BoosterStop);
			}
			else
			{
				loco->LocationReached(feedbackID);
			}

			this->state = state;
			this->stateDelayed = state;
			return true;
		}

		for (auto f : feedbacks)
		{
			datamodel::Feedback* feedback = manager->GetFeedbackUnlocked(f);
			if (feedback == nullptr)
			{
				continue;
			}
			if (feedback->GetState() != FeedbackStateFree)
			{
				return false;
			}
		}
		this->state = FeedbackStateFree;
		if (this->GetLoco() == LocoNone)
		{
			this->stateDelayed = FeedbackStateFree;
			this->locoIdDelayed = LocoNone;
		}
		return true;
	}

	bool Track::AddStreet(Street* street)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto s : streets)
		{
			if (s == street)
			{
				return false;
			}
		}
		streets.push_back(street);
		return true;
	}

	bool Track::RemoveStreet(Street* street)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		size_t sizeBefore = streets.size();
		streets.erase(std::remove(streets.begin(), streets.end(), street), streets.end());
		size_t sizeAfter = streets.size();
		return sizeBefore > sizeAfter;
	}

	Track::selectStreetApproach_t Track::GetSelectStreetApproachCalculated() const
	{
		if (selectStreetApproach == SelectStreetSystemDefault)
		{
			return manager->GetSelectStreetApproach();
		}
		return selectStreetApproach;
	}

	bool Track::GetValidStreets(const Loco* loco, const bool allowLocoTurn, std::vector<Street*>& validStreets) const
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto street : streets)
		{
			if (street->FromTrackDirection(objectID, locoDirection, loco, allowLocoTurn))
			{
				validStreets.push_back(street);
			}
		}
		OrderValidStreets(validStreets);
		return true;
	}

	void Track::OrderValidStreets(vector<Street*>& validStreets) const
	{
		switch (GetSelectStreetApproachCalculated())
		{

			case Track::SelectStreetRandom:
				std::random_shuffle(validStreets.begin(), validStreets.end());
				break;

			case Track::SelectStreetMinTrackLength:
				std::sort(validStreets.begin(), validStreets.end(), Street::CompareShortest);
				break;

			case Track::SelectStreetLongestUnused:
				std::sort(validStreets.begin(), validStreets.end(), Street::CompareLastUsed);
				break;

			case Track::SelectStreetDoNotCare:
			default:
				// do nothing
				break;
		}
	}
} // namespace datamodel

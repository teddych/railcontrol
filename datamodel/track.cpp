#include <algorithm>
#include <map>
#include <sstream>
#include <string>

#include "datamodel/feedback.h"
#include "datamodel/track.h"
#include "manager.h"
#include "util.h"

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
			<< ";locoDirection=" << static_cast<int>(locoDirection);
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
		string objectType = GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Track") != 0)
		{
			return false;
		}
		type = static_cast<trackType_t>(GetBoolMapEntry(arguments, "type", TrackTypeStraight));
		string feedbackStrings = GetStringMapEntry(arguments, "feedbacks");
		vector<string> feedbackStringVector;
		str_split(feedbackStrings, ",", feedbackStringVector);
		for (auto feedbackString : feedbackStringVector)
		{
			feedbacks.push_back(Util::StringToInteger(feedbackString));
		}
		selectStreetApproach = static_cast<selectStreetApproach_t>(GetIntegerMapEntry(arguments, "selectstreetapproach", SelectStreetSystemDefault));
		state = static_cast<feedbackState_t>(GetBoolMapEntry(arguments, "state", FeedbackStateFree));
		locoDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "locoDirection", DirectionRight));
		return true;
	}

	bool Track::Reserve(const locoID_t locoID)
	{
		if (state != FeedbackStateFree)
		{
			return false;
		}
		return LockableItem::Reserve(locoID);
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
		manager->TrackPublishState(this);
		return ret;
	}

	bool Track::FeedbackState(const feedbackID_t feedbackID, const feedbackState_t state)
	{
		feedbackState_t oldState = this->state;
		bool ret = FeedbackStateInternal(feedbackID, state);
		if (ret == true && oldState != state)
		{
			manager->TrackPublishState(this);
		}
		return ret;
	}

	bool Track::FeedbackStateInternal(const feedbackID_t feedbackID, const feedbackState_t state)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state == FeedbackStateOccupied)
		{
			Loco* loco = manager->GetLoco(GetLoco());
			if (loco == nullptr)
			{
				manager->Booster(ControlTypeInternal, BoosterStop);
			}
			else
			{
				loco->LocationReached(feedbackID);
			}

			this->state = state;
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

	bool Track::GetValidStreets(const Loco* loco, std::vector<Street*>& validStreets) const
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto street : streets)
		{
			if (street->FromTrackDirection(objectID, locoDirection, loco))
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

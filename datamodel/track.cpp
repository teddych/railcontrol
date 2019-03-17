#include <algorithm>
#include <map>
#include <sstream>
#include <string>

#include <datamodel/feedback.h>
#include <datamodel/track.h>
#include <manager.h>

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
		ss << "objectType=Track;" << LayoutItem::Serialize()
			<< ";type=" << static_cast<int>(type)
			<< ";feedbacks=" << feedbackString
			<< ";state=" << static_cast<int>(state)
			<< ";lockState=" << static_cast<int>(lockState)
			<< ";locoID=" << static_cast<int>(locoID)
			<< ";locoDirection=" << static_cast<int>(locoDirection);
		return ss.str();
	}

	bool Track::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		parseArguments(serialized, arguments);
		LayoutItem::Deserialize(arguments);
		width = Width1;
		visible = VisibleYes;
		if (!arguments.count("objectType") || arguments.at("objectType").compare("Track") != 0)
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
		state = static_cast<feedbackState_t>(GetBoolMapEntry(arguments, "state", FeedbackStateFree));
		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		locoID = GetIntegerMapEntry(arguments, "locoID", LocoNone);
		locoDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "locoDirection", DirectionLeft));
		return true;
	}

	bool Track::FeedbackState(const feedbackID_t feedbackID, const feedbackState_t state)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (state != FeedbackStateFree)
		{
			this->state = state;
			return true;
		}
		for (auto f : feedbacks)
		{
			datamodel::Feedback* feedback = manager->GetFeedback(f);
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

	bool Track::Reserve(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoID)
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
		this->locoID = locoID;
		return true;
	}

	bool Track::Lock(const locoID_t locoID)
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
		lockState = LockStateHardLocked;
		return true;
	}

	bool Track::Release(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (this->locoID != locoID && locoID != LocoNone)
		{
			return false;
		}
		this->locoID = LocoNone;
		lockState = LockStateFree;
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

	bool Track::ValidStreets(std::vector<Street*>& validStreets)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto street : streets)
		{
			if (street->fromTrackDirection(objectID, locoDirection))
			{
				validStreets.push_back(street);
			}
		}
		return true;
	}
} // namespace datamodel

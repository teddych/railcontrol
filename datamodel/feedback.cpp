#include <map>
#include <sstream>

#include "datamodel/feedback.h"
#include "manager.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Feedback::Serialize() const
	{
		stringstream ss;
		ss << "objectType=Feedback;" << LayoutItem::Serialize()
			<< ";controlID=" << static_cast<int>(controlID)
			<< ";pin=" << static_cast<int>(pin)
			<< ";inverted=" << static_cast<int>(inverted)
			<< ";state=" << static_cast<int>(stateCounter > 0)
			<< ";track=" << static_cast<int>(trackID);
		return ss.str();
	}

	bool Feedback::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = Utils::Utils::GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Feedback") != 0)
		{
			return false;
		}
		LayoutItem::Deserialize(arguments);
		controlID = Utils::Utils::GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		pin = Utils::Utils::GetIntegerMapEntry(arguments, "pin");
		inverted = Utils::Utils::GetBoolMapEntry(arguments, "inverted", false);
		stateCounter = Utils::Utils::GetBoolMapEntry(arguments, "state", FeedbackStateFree) ? MaxStateCounter : 0;
		trackID = static_cast<trackID_t>(Utils::Utils::GetIntegerMapEntry(arguments, "track", TrackNone));
		return true;
	}

	void Feedback::SetState(const feedbackState_t newState)
	{
		feedbackState_t state = static_cast<feedbackState_t>(newState != inverted);
		if (state == FeedbackStateFree)
		{
			if (stateCounter < MaxStateCounter)
			{
				return;
			}
			stateCounter = MaxStateCounter - 1;
			return;
		}

		if (stateCounter == MaxStateCounter)
		{
			return;
		}
		stateCounter = MaxStateCounter;
		manager->FeedbackState(this);
		UpdateTrackState(FeedbackStateOccupied);
	}

	void Feedback::UpdateTrackState(const feedbackState_t state)
	{
		Track* track = manager->GetTrack(trackID);
		if (track == nullptr)
		{
			return;
		}
		track->SetFeedbackState(objectID, state);
		return;
	}

	void Feedback::Debounce()
	{
		if (stateCounter == MaxStateCounter || stateCounter == 0)
		{
			return;
		}

		--stateCounter;
		if (stateCounter != 0)
		{
			return;
		}
		manager->FeedbackState(this);
		UpdateTrackState(FeedbackStateFree);
	}
} // namespace datamodel


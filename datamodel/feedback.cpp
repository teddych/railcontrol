#include <map>
#include <sstream>

#include "datamodel/feedback.h"
#include "manager.h"

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
			<< ";state=" << static_cast<int>(state)
			<< ";track=" << static_cast<int>(trackID);
		return ss.str();
	}

	bool Feedback::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") != 1 || arguments.at("objectType").compare("Feedback") != 0)
		{
			return false;
		}
		LayoutItem::Deserialize(arguments);
		controlID = GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		pin = GetIntegerMapEntry(arguments, "pin");
		inverted = GetBoolMapEntry(arguments, "inverted", false);
		state = static_cast<feedbackState_t>(GetBoolMapEntry(arguments, "state", FeedbackStateFree));
		trackID = static_cast<trackID_t>(GetIntegerMapEntry(arguments, "track", TrackNone));
		return true;
	}

	bool Feedback::SetState(const feedbackState_t newState)
	{
		state = static_cast<feedbackState_t>(newState != inverted);

		Track* track = manager->GetTrack(trackID);
		if (track == nullptr)
		{
			return true;
		}
		track->FeedbackState(objectID, state);
		return true;
	}
} // namespace datamodel


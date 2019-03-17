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

	bool Feedback::Release(const locoID_t locoID)
	{
		if (locoID != this->locoID)
		{
			return false;
		}
		this->locoID = LocoNone;
		return true;
	}

	bool Feedback::SetLoco(const locoID_t locoID)
	{
		// FIXME: should check if already a loco is set / basically is done by street
		/*
		if (locoID == LOCO_NONE) {
			return false;
		}
		*/
		this->locoID = locoID;
		return true;
	}

	bool Feedback::SetState(const feedbackState_t state)
	{
		this->state = static_cast<feedbackState_t>((state ^ inverted) & 0x01);

		Loco* loco = manager->GetLoco(locoID);

		manager->TrackSetFeedbackState(trackID, this->objectID, this->state, loco == nullptr ? "" : loco->Name());

		if (this->state == FeedbackStateFree)
		{
			return true;
		}

		if (loco == nullptr)
		{
			return false;
		}

		loco->DestinationReached();
		return true;
	}

} // namespace datamodel


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
		ss << "objectType=Feedback;" << LayoutItem::Serialize() << ";controlID=" << (int) controlID << ";pin=" << (int) pin << ";inverted=" << (int) inverted << ";state=" << (int) state;
		return ss.str();
	}

	bool Feedback::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Feedback") == 0)
		{
			LayoutItem::Deserialize(arguments);
			controlID = GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
			pin = GetIntegerMapEntry(arguments, "pin");
			inverted = GetBoolMapEntry(arguments, "inverted", false);
			state = static_cast<feedbackState_t>(GetBoolMapEntry(arguments, "state", FeedbackStateFree));
			return true;
		}
		return false;
	}

	bool Feedback::release(const locoID_t locoID)
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
		if (state == FeedbackStateFree)
		{
			return true;
		}

		if (locoID == LocoNone)
		{
			return true;
		}

		manager->getLoco(locoID)->destinationReached();

		return true;
	}

} // namespace datamodel


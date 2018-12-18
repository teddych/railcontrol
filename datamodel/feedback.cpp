#include <map>
#include <sstream>

#include "datamodel/feedback.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Feedback::Feedback(Manager* manager,
		const feedbackID_t feedbackID,
		const std::string& name,
		const layoutPosition_t x,
		const layoutPosition_t y,
		const layoutPosition_t z,
		const controlID_t controlID,
		const feedbackPin_t pin,
		bool inverted)
	:	LayoutItem(feedbackID, name, x, y, z, Width1, Height1, Rotation0),
		controlID(controlID),
		pin(pin),
		manager(manager),
		state(FeedbackStateFree),
		locoID(LocoNone),
		inverted(inverted)
	{
	}

	Feedback::Feedback(Manager* manager, const std::string& serialized)
	:	manager(manager),
		locoID(LocoNone)
	{
		deserialize(serialized);
	}

	std::string Feedback::serialize() const
	{
		stringstream ss;
		ss << "objectType=Feedback;" << LayoutItem::serialize() << ";controlID=" << (int) controlID << ";pin=" << (int) pin << ";inverted=" << (int) inverted << ";state=" << (int) state;
		return ss.str();
	}

	bool Feedback::deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Feedback") == 0)
		{
			LayoutItem::deserialize(arguments);
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

	bool Feedback::setLoco(const locoID_t locoID)
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

	bool Feedback::setState(const feedbackState_t state)
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


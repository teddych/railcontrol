#include <map>
#include <sstream>

#include "feedback.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Feedback::Feedback(Manager* manager, const feedbackID_t feedbackID, const std::string& name, const controlID_t controlID, const feedbackPin_t pin, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z) :
		LayoutItem(feedbackID, name, ROTATION_0, /*width*/ 1, /*height*/ 1, x, y, z),
		manager(manager),
		controlID(controlID),
		pin(pin),
		state(FEEDBACK_STATE_FREE),
		locoID(LOCO_NONE) {
	}

	Feedback::Feedback(Manager* manager, const std::string& serialized) :
		manager(manager),
		locoID(LOCO_NONE) {
		deserialize(serialized);
	}

	std::string Feedback::serialize() const {
		stringstream ss;
		ss << "objectType=Feedback;" << LayoutItem::serialize() << ";controlID=" << (int)controlID << ";pin=" << (int)pin << ";state=" << (int)state;
		return ss.str();
	}

	bool Feedback::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Feedback") == 0) {
			LayoutItem::deserialize(arguments);
			if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
			if (arguments.count("pin")) pin = stoi(arguments.at("pin"));
			if (arguments.count("state")) state = stoi(arguments.at("state"));
			return true;
		}
		return false;
	}

	bool Feedback::setLoco(const locoID_t locoID) {
		if (locoID != LOCO_NONE) return false;
		this->locoID = locoID;
		return true;
	}

	bool Feedback::setState(const feedbackState_t state) {
		if (this->state == state) return false;

		this->state = state;
		if (!state) return true;

		if (!locoID) return true;

		manager->getLoco(locoID)->destinationReached();

		return true;
	}

} // namespace datamodel


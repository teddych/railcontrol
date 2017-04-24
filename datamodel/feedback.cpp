#include <map>
#include <sstream>

#include "feedback.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Feedback::Feedback(feedbackID_t feedbackID, std::string name, controlID_t controlID, feedbackPin_t pin, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(ROTATION_0, /*width*/ 1, /*height*/ 1, x, y, z),
		feedbackID(feedbackID),
		name(name),
		controlID(controlID),
		pin(pin),
		state(FEEDBACK_STATE_FREE) {
	}

	Feedback::Feedback(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Feedback::serialize() const {
		stringstream ss;
		ss << "objectType=Feedback;feedbackID=" << (int)feedbackID << ";name=" << name << ";controlID=" << (int)controlID << ";pin=" << (int)pin << ";state=" << (int)state << LayoutItem::serialize();
		return ss.str();
	}

	bool Feedback::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Feedback") == 0) {
			LayoutItem::deserialize(arguments);
			if (arguments.count("feedbackID")) feedbackID = stoi(arguments.at("feedbackID"));
			if (arguments.count("name")) name = arguments.at("name");
			if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
			if (arguments.count("pin")) pin = stoi(arguments.at("pin"));
			if (arguments.count("state")) state = stoi(arguments.at("state"));
			return true;
		}
		return false;
	}

} // namespace datamodel


#include "feedback.h"

namespace datamodel {

	Feedback::Feedback(feedbackID_t feedbackID, std::string name, controlID_t controlID, feedbackPin_t pin, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(ROTATION_0, /*width*/ 1, /*height*/ 1, x, y, z),
		feedbackID(feedbackID),
		name(name),
		controlID(controlID),
		pin(pin),
		state(FEEDBACK_STATE_FREE) {
	}

	/*
	Feedback::Feedback(const std::string serialized) {
		deserialize(serialized);
	}
	*/

	std::string Feedback::serialize() const {
		return "";
	}

	bool Feedback::deserialize(const std::string serialized) {
		return true;
	}

} // namespace datamodel


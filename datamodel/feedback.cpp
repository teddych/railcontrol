#include "feedback.h"

namespace datamodel {

	Feedback::Feedback(feedbackID_t feedbackID, std::string name, controlID_t controlID, feedbackPin_t pin) :
		feedbackID(feedbackID),
		name(name),
		controlID(controlID),
		pin(pin),
		state(FEEDBACK_STATE_FREE) {
	}

} // namespace datamodel


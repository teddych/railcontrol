#include "feedback.h"

namespace datamodel {

	Feedback::Feedback(feedbackID_t feedbackID, std::string name, controlID_t controlID, feedbackPin_t pin, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		feedbackID(feedbackID),
		name(name),
		controlID(controlID),
		pin(pin),
		state(FEEDBACK_STATE_FREE) {

		height = 1;
		width = 1;
		rotation = ROTATION_0;
		posX = x;
		posY = y;
		posZ = z;
	}

} // namespace datamodel


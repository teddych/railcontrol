#ifndef DATAMODEL_FEEDBACK_H
#define DATAMODEL_FEEDBACK_H

#include <string>

#include "datatypes.h"

namespace datamodel {

	class Feedback {
		public:
			Feedback(feedbackID_t feedbackID, std::string name, controlID_t controlID, feedbackPin_t pin);

			feedbackID_t feedbackID;
			std::string name;
			controlID_t controlID;
			feedbackPin_t pin;
			feedbackState_t state;
	};

} // namespace datamodel

#endif // DATAMODEL_FEEDBACK_H

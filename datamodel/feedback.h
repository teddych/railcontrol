#ifndef DATAMODEL_FEEDBACK_H
#define DATAMODEL_FEEDBACK_H

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

namespace datamodel {

	class Feedback : public LayoutItem {
		public:
			Feedback(feedbackID_t feedbackID, std::string name, controlID_t controlID, feedbackPin_t pin, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z);
			Feedback(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			feedbackID_t feedbackID;
			std::string name;
			controlID_t controlID;
			feedbackPin_t pin;
			feedbackState_t state;
	};

} // namespace datamodel

#endif // DATAMODEL_FEEDBACK_H

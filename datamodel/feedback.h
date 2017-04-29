#ifndef DATAMODEL_FEEDBACK_H
#define DATAMODEL_FEEDBACK_H

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

namespace datamodel {

	class Feedback : public LayoutItem {
		public:
			Feedback(const feedbackID_t feedbackID, const std::string& name, const controlID_t controlID, const feedbackPin_t pin, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z);
			Feedback(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			controlID_t controlID;
			feedbackPin_t pin;
			feedbackState_t state;
	};

} // namespace datamodel

#endif // DATAMODEL_FEEDBACK_H

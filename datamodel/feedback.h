#ifndef DATAMODEL_FEEDBACK_H
#define DATAMODEL_FEEDBACK_H

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

class Manager;

namespace datamodel {

	class Feedback : public LayoutItem {
		public:
			Feedback(Manager* manager, const feedbackID_t feedbackID, const std::string& name, const controlID_t controlID, const feedbackPin_t pin, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z);
			Feedback(Manager* manager, const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			bool setLoco(const locoID_t locoID);
			bool setState(const feedbackState_t state);

		private:
			Manager* manager;
			controlID_t controlID;
			feedbackPin_t pin;
			feedbackState_t state;
			locoID_t locoID;
	};

} // namespace datamodel

#endif // DATAMODEL_FEEDBACK_H

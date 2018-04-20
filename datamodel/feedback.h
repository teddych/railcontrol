#pragma once

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

class Manager;

namespace datamodel {

	class Feedback : public LayoutItem {
		public:
			Feedback(Manager* manager, const feedbackID_t feedbackID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const controlID_t controlID, const feedbackPin_t pin);
			Feedback(Manager* manager, const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;
			virtual std::string layoutType() const { return "feedback"; };

			bool setLoco(const locoID_t locoID);
			bool setState(const feedbackState_t state);

			controlID_t controlID;
			feedbackPin_t pin;

		private:
			Manager* manager;
			feedbackState_t state;
			locoID_t locoID;
	};

} // namespace datamodel


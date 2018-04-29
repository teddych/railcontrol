#pragma once

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

class Manager;

namespace datamodel {

	class Feedback : public LayoutItem {
		public:
			Feedback(Manager* manager, const feedbackID_t feedbackID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const controlID_t controlID, const feedbackPin_t pin, bool inverted);
			Feedback(Manager* manager, const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;
			virtual std::string layoutType() const { return "feedback"; };

			bool release(const locoID_t locoID);
			bool setLoco(const locoID_t locoID);
			locoID_t getLoco() const { return locoID; }
			bool setState(const feedbackState_t state);
			feedbackState_t getState() const { return state; }

			// make privete
			controlID_t controlID;
			feedbackPin_t pin;

		private:
			Manager* manager;
			feedbackState_t state;
			locoID_t locoID;
			bool inverted;
	};

} // namespace datamodel


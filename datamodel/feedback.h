#pragma once

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

class Manager;

namespace datamodel
{
	class Feedback : public LayoutItem
	{
		public:
			Feedback(Manager* manager,
				const feedbackID_t feedbackID,
				const std::string& name,
				const visible_t visible,
				const layoutPosition_t x,
				const layoutPosition_t y,
				const layoutPosition_t z,
				const controlID_t controlID,
				const feedbackPin_t pin,
				bool inverted)
			:	LayoutItem(feedbackID, name, visible, x, y, z, Width1, Height1, Rotation0),
				controlID(controlID),
				pin(pin),
				manager(manager),
				state(FeedbackStateFree),
				locoID(LocoNone),
				inverted(inverted)
			{
			}

			Feedback(Manager* manager, const std::string& serialized)
			:	manager(manager),
				locoID(LocoNone)
			{
				Deserialize(serialized);
			}

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "feedback"; };

			void Inverted(const bool inverted) { this->inverted = inverted; }
			bool IsInverted() const { return inverted; }

			bool release(const locoID_t locoID);
			bool SetLoco(const locoID_t locoID);
			locoID_t GetLoco() const { return locoID; }
			bool SetState(const feedbackState_t state);
			feedbackState_t GetState() const { return state; }

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


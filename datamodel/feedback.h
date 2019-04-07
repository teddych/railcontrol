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
				const bool inverted)
			:	LayoutItem(feedbackID, name, visible, x, y, z, Width1, Height1, Rotation0),
				controlID(controlID),
				pin(pin),
				manager(manager),
				inverted(inverted),
				trackID(TrackNone),
				stateCounter(0)
			{
			}

			Feedback(Manager* manager, const std::string& serialized)
			:	manager(manager)
			{
				Deserialize(serialized);
			}

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "feedback"; };

			void Inverted(const bool inverted) { this->inverted = inverted; }
			bool IsInverted() const { return inverted; }

			void SetState(const feedbackState_t state);
			feedbackState_t GetState() const { return static_cast<feedbackState_t>(stateCounter > 0); }
			void Debounce();
			void SetTrack(const trackID_t trackID) { this->trackID = trackID; }
			trackID_t GetTrack() const { return trackID; }

			// FIXME: make privete
			controlID_t controlID;
			feedbackPin_t pin;

		private:
			void UpdateTrackState(const feedbackState_t state);

			Manager* manager;
			bool inverted;
			trackID_t trackID;
			unsigned char stateCounter;
			const unsigned char MaxStateCounter = 10;
	};

} // namespace datamodel


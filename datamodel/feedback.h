#pragma once

#include <string>

#include "datatypes.h"
#include "datamodel/layout_item.h"

class Manager;

namespace datamodel
{
	class Feedback : public LayoutItem
	{
		public:
			Feedback(Manager* manager,
				const feedbackID_t feedbackID)
			:	LayoutItem(feedbackID),
			 	manager(manager),
			 	inverted(false),
			 	trackID(TrackNone),
				stateCounter(0)
			{
			}

			Feedback(Manager* manager, const std::string& serialized)
			:	manager(manager)
			{
				Deserialize(serialized);
			}

			objectType_t GetObjectType() const { return ObjectTypeFeedback; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "feedback"; };

			void SetInverted(const bool inverted) { this->inverted = inverted; }
			bool GetInverted() const { return inverted; }

			void SetState(const feedbackState_t state);
			feedbackState_t GetState() const { return static_cast<feedbackState_t>(stateCounter > 0); }
			void Debounce();
			void SetControlID(const controlID_t controlID) { this->controlID = controlID; }
			controlID_t GetControlID() const { return controlID; }
			void SetPin(const feedbackPin_t pin) { this->pin = pin; }
			feedbackPin_t GetPin() const { return pin; }
			void SetTrack(const trackID_t trackID) { this->trackID = trackID; }
			trackID_t GetTrack() const { return trackID; }

		private:
			controlID_t controlID;
			feedbackPin_t pin;

			void UpdateTrackState(const feedbackState_t state);

			Manager* manager;
			bool inverted;
			trackID_t trackID;
			unsigned char stateCounter;
			const unsigned char MaxStateCounter = 10;
	};

} // namespace datamodel


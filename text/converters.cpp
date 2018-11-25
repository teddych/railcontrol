#include "text/converters.h"

using std::string;

namespace text
{
	void Converters::lockStatus(const lockState_t state, string& stateText) {
		switch (state) {
			case LockStateFree:
				stateText.assign("free");
				break;
			case LockStateReserved:
				stateText.assign("reserved");
				break;
			case LockStateSoftLocked:
				stateText.assign("soft locked");
				break;
			case LockStateHardLocked:
				stateText.assign("locked");
				break;
			default:
				stateText.assign("unknown");
		}
	}

	void Converters::accessoryStatus(const accessoryState_t state, string& onText)
	{
		onText.assign(state == AccessoryStateOn ? "green" : "red");
	}

	void Converters::feedbackStatus(const feedbackState_t state, string& stateText) {
		switch (state) {
			case FeedbackStateFree:
				stateText.assign("free");
				break;
			case FeedbackStateOccupied:
				stateText.assign("occupied");
				break;
			default:
				stateText.assign("unknown");
		}
	}

	void Converters::switchStatus(const switchState_t state, string& stateText)
	{
		stateText.assign(state == SwitchStateStraight ?"straight" : "turnout");
	}
}

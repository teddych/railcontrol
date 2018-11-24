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

	void Converters::accessoryStatus(const accessoryState_t state, string& onText) {
		switch (state) {
			case AccessoryStateOff:
				onText.assign("red");
				break;
			case AccessoryStateOn:
				onText.assign("green");
				break;
			default:
				onText.assign("unknown");
		}
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

	void Converters::switchStatus(const switchState_t state, string& stateText) {
		switch (state) {
			case SwitchStateStraight:
				stateText.assign("straight");
				break;
			case SwitchStateTurnout:
				stateText.assign("turnout");
				break;
			default:
				stateText.assign("unknown");
		}
	}
}

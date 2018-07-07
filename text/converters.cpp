#include "converters.h"

using std::string;

namespace text {
	void Converters::locoStatus(const locoState_t state, string& stateText) {
		switch (state) {
			case LocoStateManual:
				stateText.assign("manual");
				break;
			case LocoStateOff:
				stateText.assign("off");
				break;
			case LocoStateSearching:
				stateText.assign("searching");
				break;
			case LocoStateRunning:
				stateText.assign("running");
				break;
			case LocoStateStopping:
				stateText.assign("stopping");
				break;
			case LocoStateError:
				stateText.assign("error");
				break;
			default:
				stateText.assign("unknown");
		}
	}

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

	void Converters::accessoryStatus(const accessoryState_t state, string& colorText, string& onText) {
		// calculate color
		switch (state >> 1) {
			case AccessoryColorRed:
				colorText.assign("red");
				break;
			case AccessoryColorGreen:
				colorText.assign("green");
				break;
			case AccessoryColorYellow:
				colorText.assign("yellow");
				break;
			case AccessoryColorWhite:
				colorText.assign("white");
				break;
			default:
				colorText.assign("unknown");
		}
		// calculate on
		switch (state & 0x01) {
			case AccessoryStateOff:
				onText.assign("off");
				break;
			case AccessoryStateOn:
				onText.assign("on");
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
		switch (state >> 1) {
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

#include "converters.h"

using std::string;

namespace text {
	void Converters::locoStatus(const locoState_t state, string& stateText) {
		switch (state) {
			case LOCO_STATE_MANUAL:
				stateText.assign("manual");
				break;
			case LOCO_STATE_OFF:
				stateText.assign("off");
				break;
			case LOCO_STATE_SEARCHING:
				stateText.assign("searching");
				break;
			case LOCO_STATE_RUNNING:
				stateText.assign("running");
				break;
			case LOCO_STATE_STOPPING:
				stateText.assign("stopping");
				break;
			case LOCO_STATE_ERROR:
				stateText.assign("error");
				break;
			default:
				stateText.assign("unknown");
		}
	}

	void Converters::lockStatus(const lockState_t state, string& stateText) {
		switch (state) {
			case LOCK_STATE_FREE:
				stateText.assign("free");
				break;
			case LOCK_STATE_RESERVED:
				stateText.assign("reserved");
				break;
			case LOCK_STATE_SOFT_LOCKED:
				stateText.assign("soft locked");
				break;
			case LOCK_STATE_HARD_LOCKED:
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
			case FEEDBACK_STATE_FREE:
				stateText.assign("free");
				break;
			case FEEDBACK_STATE_OCCUPIED:
				stateText.assign("occupied");
				break;
			default:
				stateText.assign("unknown");
		}
	}

	void Converters::switchStatus(const switchState_t state, string& stateText) {
		switch (state >> 1) {
			case SWITCH_STRAIGHT:
				stateText.assign("straight");
				break;
			case SWITCH_TURNOUT:
				stateText.assign("turnout");
				break;
			default:
				stateText.assign("unknown");
		}
	}
}

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

	void Converters::blockStatus(const blockState_t state, string& stateText) {
		switch (state) {
			case BLOCK_STATE_FREE:
				stateText.assign("free");
				break;
			case BLOCK_STATE_RESERVED:
				stateText.assign("reserved");
				break;
			case BLOCK_STATE_LOCKED:
				stateText.assign("locked");
				break;
			default:
				stateText.assign("unknown");
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

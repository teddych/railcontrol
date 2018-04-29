#include "converters.h"

using std::string;

namespace text {
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

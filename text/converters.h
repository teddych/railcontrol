#pragma once

#include <string>

#include "datatypes.h"

namespace text {
	class Converters {
		public:
			static void locoStatus(const locoState_t state, std::string& stateText);
			static void accessoryStatus(const accessoryState_t state, std::string& colorText, std::string& onText);
			static void blockStatus(const lockState_t state, std::string& stateText);
			static void feedbackStatus(const feedbackState_t state, std::string& stateText);
			static void streetStatus(const lockState_t state, std::string& stateText) { blockStatus(state, stateText); }
			static void switchStatus(const switchState_t state, std::string& stateText);
	};
}

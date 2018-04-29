#pragma once

#include <string>

#include "datatypes.h"

namespace text {
	class Converters {
		public:
			static void blockStatus(const blockState_t state, std::string& stateText);
			static void streetStatus(const streetState_t state, std::string& stateText) { blockStatus(state, stateText); }
			static void switchStatus(const switchState_t state, std::string& stateText);
	};
}

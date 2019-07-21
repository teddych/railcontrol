#pragma once

#include <string>

#include "datatypes.h"

namespace text
{
	class Converters
	{
		public:
			static void accessoryStatus(const accessoryState_t state, std::string& onText);
			static void switchStatus(const switchState_t state, std::string& stateText);
			static void signalStatus(const signalState_t state, std::string& stateText);
	};
}

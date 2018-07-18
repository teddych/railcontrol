#pragma once

#include <string>

#include "webserver/HtmlTagInput.h"

namespace webserver
{
	class HtmlTagInputHidden : public HtmlTagInput
	{
		public:
			HtmlTagInputHidden(const std::string& name, const std::string& value)
			: HtmlTagInput("hidden", name, value) {};
	};
};


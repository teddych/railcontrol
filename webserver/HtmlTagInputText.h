#pragma once

#include <string>

#include "webserver/HtmlTagInput.h"

namespace webserver
{
	class HtmlTagInputText : public HtmlTagInput
	{
		public:
			HtmlTagInputText(const std::string& name, const std::string& value)
			: HtmlTagInput("text", name, value) {};
	};
};


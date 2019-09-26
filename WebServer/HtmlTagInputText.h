#pragma once

#include <string>

#include "WebServer/HtmlTagInput.h"

namespace WebServer
{
	class HtmlTagInputText : public HtmlTagInput
	{
		public:
			HtmlTagInputText(const std::string& name, const std::string& value)
			: HtmlTagInput("text", name, value) {};
	};
};


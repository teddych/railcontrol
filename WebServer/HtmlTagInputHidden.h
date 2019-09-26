#pragma once

#include <string>

#include "WebServer/HtmlTagInput.h"

namespace WebServer
{
	class HtmlTagInputHidden : public HtmlTagInput
	{
		public:
			HtmlTagInputHidden(const std::string& name, const std::string& value)
			: HtmlTagInput("hidden", name, value) {};
	};
};


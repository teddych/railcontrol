#pragma once

#include <string>

#include "WebServer/HtmlTag.h"

namespace WebServer
{
	class HtmlTagInput : public HtmlTag
	{
		public:
			HtmlTagInput(const std::string& type, const std::string& name, const std::string& value = "");
	};
};


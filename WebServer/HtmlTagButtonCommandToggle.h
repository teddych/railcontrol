#pragma once

#include <atomic>
#include <map>
#include <string>

#include "WebServer/HtmlTagButton.h"

namespace WebServer
{
	class HtmlTagButtonCommandToggle : public HtmlTagButton
	{
		public:
			HtmlTagButtonCommandToggle(const std::string& value, const std::string& command, const bool on, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
	};
};

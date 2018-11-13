#pragma once

#include <atomic>
#include <map>
#include <string>

#include "webserver/HtmlTagButton.h"

namespace webserver
{
	class HtmlTagButtonCommandToggle : public HtmlTagButton
	{
		public:
			HtmlTagButtonCommandToggle(const std::string& value, const std::string& command, const bool on, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
	};
};

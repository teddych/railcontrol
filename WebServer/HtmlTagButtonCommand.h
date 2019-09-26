#pragma once

#include <atomic>
#include <map>
#include <string>

#include "WebServer/HtmlTagButton.h"

namespace WebServer
{
	class HtmlTagButtonCommand : public HtmlTagButton
	{
		public:
			HtmlTagButtonCommand(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>(), const std::string& additionalOnClick = "");
	};
};

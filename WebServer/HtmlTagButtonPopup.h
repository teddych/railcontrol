#pragma once

#include <atomic>
#include <map>
#include <string>

#include "WebServer/HtmlTagButton.h"

namespace WebServer
{
	class HtmlTagButtonPopup : public HtmlTagButton
	{
		public:
			HtmlTagButtonPopup(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
	};
};

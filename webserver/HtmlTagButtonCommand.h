#pragma once

#include <atomic>
#include <map>
#include <string>

#include "webserver/HtmlTagInput.h"

namespace webserver
{
	class HtmlTagButtonCommand : public HtmlTag
	{
		private:
			static std::atomic<unsigned int> buttonID;
			std::string commandID;
			HtmlTagInput inputTag;
			HtmlTag javascriptTag;

		public:
			HtmlTagButtonCommand(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
			friend std::ostream& operator<<(std::ostream& stream, const HtmlTagButtonCommand& tag);
	};
};

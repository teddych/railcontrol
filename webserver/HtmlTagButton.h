#pragma once

#include <atomic>
#include <map>
#include <string>

#include "webserver/HtmlTagInput.h"

namespace webserver
{
	class HtmlTagButton : public HtmlTag
	{
		public:
			HtmlTagButton(const std::string& value, const std::string& command, const std::string& javascript = "");
			friend std::ostream& operator<<(std::ostream& stream, const HtmlTagButton& tag);

		protected:
			static std::atomic<unsigned int> buttonID;
			std::string commandID;
			HtmlTagInput inputTag;
			HtmlTag javascriptTag;
	};
};

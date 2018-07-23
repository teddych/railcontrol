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
			void AddJavaScript(const std::string& content);
			virtual void AddAttribute(const std::string& name, const std::string& value);
			virtual void AddChildTag(const HtmlTag& child);
			virtual void AddContent(const std::string& content);

		protected:
			static std::atomic<unsigned int> buttonID;
			const std::string commandID;
	};
};

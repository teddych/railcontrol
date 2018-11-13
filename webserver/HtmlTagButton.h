#pragma once

#include <atomic>
#include <map>
#include <string>

#include "webserver/HtmlTagInput.h"
#include "webserver/HtmlTagJavascript.h"

namespace webserver
{
	class HtmlTagButton : public HtmlTag
	{
		public:
			HtmlTagButton(const std::string& value, const std::string& command);

			HtmlTag AddJavaScript(const std::string& content)
			{
				return AddChildTag(HtmlTagJavascript(content));
			}

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}

			virtual HtmlTag AddClass(const std::string& value) override
			{
				childTags[0].AddClass(value);
				return *this;
			}

		protected:
			const std::string commandID;
	};
};

#pragma once

#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagInput : public HtmlTag
	{
		private:
			HtmlTag labelTag;
			HtmlTag inputTag;

		public:
			HtmlTagInput(const std::string& type, const std::string& name, const std::string& value = "", const std::string& label = "");
			virtual void AddAttribute(const std::string& name, const std::string& value) { inputTag.AddAttribute(name, value); }
			virtual void AddChildTag(const HtmlTag& child) { inputTag.AddChildTag(child); }
			virtual void AddContent(const std::string& content) { inputTag.AddContent(content); }

			friend std::ostream& operator<<(std::ostream& stream, const HtmlTagInput& tag);
	};
};


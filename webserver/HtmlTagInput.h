#pragma once

#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagInput : public HtmlTag
	{
		public:
			HtmlTagInput(const std::string& type, const std::string& name, const std::string& value = "", const std::string& label = "");
			virtual void AddAttribute(const std::string& name, const std::string& value);
			virtual void AddChildTag(const HtmlTag& child);
			virtual void AddContent(const std::string& content);
	};
};


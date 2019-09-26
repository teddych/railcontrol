#pragma once

#include <string>

#include "WebServer/HtmlTag.h"
#include "WebServer/HtmlTagLabel.h"
#include "WebServer/HtmlTagInputInteger.h"

namespace WebServer
{
	class HtmlTagInputIntegerWithLabel : public HtmlTag
	{
		public:
			HtmlTagInputIntegerWithLabel(const std::string& name, const std::string& label, const int min, const int max)
			:	HtmlTagInputIntegerWithLabel(name, label, 0, min, max)
			{}

			HtmlTagInputIntegerWithLabel(const std::string& name, const std::string& label, const int value, const int min, const int max)
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, name));
				AddChildTag(HtmlTagInputInteger(name, value, min, max));
			}

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[1].AddAttribute(name, value);
				return *this;
			}

			virtual HtmlTag AddClass(const std::string& _class) override
			{
				childTags[1].AddClass(_class);
				return *this;
			}
	};
};


#pragma once

#include <string>

#include "webserver/HtmlTag.h"
#include "webserver/HtmlTagLabel.h"
#include "webserver/HtmlTagInputText.h"

namespace webserver
{
	class HtmlTagInputTextWithLabel : public HtmlTag
	{
		public:
			HtmlTagInputTextWithLabel(const std::string& name, const std::string& label, const std::string& value = "")
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, name));
				AddChildTag(HtmlTagInputText(name, value));
			}

			virtual void AddAttribute(const std::string& name, const std::string& value)
			{
				childTags[1].AddAttribute(name, value);
			}
	};
};


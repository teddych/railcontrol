#pragma once

#include <string>

#include "WebServer/HtmlTag.h"
#include "WebServer/HtmlTagLabel.h"
#include "WebServer/HtmlTagInputCheckbox.h"

namespace WebServer
{
	class HtmlTagInputCheckboxWithLabel : public HtmlTag
	{
		public:
			HtmlTagInputCheckboxWithLabel(const std::string& name, const std::string& label, const std::string& value, const bool checked)
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, name));
				AddChildTag(HtmlTagInputCheckbox(name, value, checked));
				AddChildTag(HtmlTag("br"));
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


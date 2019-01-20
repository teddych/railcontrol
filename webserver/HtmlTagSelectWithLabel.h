#pragma once

#include <string>

#include "webserver/HtmlTag.h"
#include "webserver/HtmlTagLabel.h"
#include "webserver/HtmlTagSelect.h"

namespace webserver
{
	class HtmlTagSelectWithLabel : public HtmlTag
	{
		public:
			HtmlTagSelectWithlabel(const std::string& name, const std::string& label, const std::map<std::string,std::string>& options, const std::string& defaultValue = "")
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, name));
				AddChildTag(HtmlTagSelect(name, options, defaultValue));
			}

			HtmlTagSelectWithlabel(const std::string& name, const std::string& label, const std::map<std::string,int>& options, const int defaultValue = 0)
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, name));
				AddChildTag(HtmlTagSelect(name, options, defaultValue));
			}

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value)
			{
				childTags[1].AddAttribute(name, value);
				return *this;
			}

			virtual HtmlTag AddClass(const std::string& _class)
			{
				childTags[1].AddClass(_class);
				return *this;
			}
	};
};


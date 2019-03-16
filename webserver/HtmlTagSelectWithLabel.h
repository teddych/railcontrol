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
			HtmlTagSelectWithLabel(const std::string& name, const std::string& label, const std::map<std::string,std::string>& options, const std::string& defaultValue = "")
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, "s_" + name));
				AddChildTag(HtmlTagSelect(name, options, defaultValue));
			}

			template<typename T> HtmlTagSelectWithLabel(const std::string& name, const std::string& label, const std::map<std::string,T>& options, const int defaultValue = 0)
			:	HtmlTag()
			{
				AddChildTag(HtmlTagLabel(label, "s_" + name));
				AddChildTag(HtmlTagSelect(name, options, defaultValue));
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


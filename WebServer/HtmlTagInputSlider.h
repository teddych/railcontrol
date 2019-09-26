#pragma once

#include <string>

#include "WebServer/HtmlTagInput.h"
#include "WebServer/HtmlTagJavascript.h"

namespace WebServer
{
	class HtmlTagInputSlider : public HtmlTag
	{
		public:
			HtmlTagInputSlider(const std::string& name, const unsigned int min, const unsigned int max, const unsigned int value = 0);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value)
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}

			virtual HtmlTag AddClass(const std::string& value)
			{
				childTags[0].AddClass(value);
				return *this;
			}
	};
};


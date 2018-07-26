#pragma once

#include <map>
#include <sstream>
#include <string>

#include "datatypes.h"
#include "util.h"
#include "webserver/HtmlTagInputSlider.h"

namespace webserver
{
	class HtmlTagInputSliderLocoSpeed : public HtmlTagInputSlider
	{
		public:
			HtmlTagInputSliderLocoSpeed(const std::string& name, const std::string& cmd, const unsigned int min, const unsigned int max, const unsigned int value, const locoID_t locoID);

			void AddJavaScript(const std::string& content)
			{
				AddChildTag(HtmlTagJavascript(content));
			}
	};
};


#pragma once

#include <map>
#include <sstream>
#include <string>

#include "DataTypes.h"
#include "WebServer/HtmlTagInputSlider.h"

namespace WebServer
{
	class HtmlTagInputSliderLocoSpeed : public HtmlTagInputSlider
	{
		public:
			HtmlTagInputSliderLocoSpeed(const std::string& name, const unsigned int min, const unsigned int max, const unsigned int value, const locoID_t locoID);

			void AddJavaScript(const std::string& content)
			{
				AddChildTag(HtmlTagJavascript(content));
			}
	};
};


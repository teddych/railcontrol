#include "WebServer/HtmlTagInputSlider.h"

namespace WebServer
{
	HtmlTagInputSlider::HtmlTagInputSlider(const std::string& name, const unsigned int min, const unsigned int max, const unsigned int value)
	: HtmlTag()
	{
		HtmlTagInput inputTag("range", name);
		inputTag.AddAttribute("min", std::to_string(min));
		inputTag.AddAttribute("max", std::to_string(max));
		inputTag.AddAttribute("value", std::to_string(value));
		AddChildTag(inputTag);
	}
};

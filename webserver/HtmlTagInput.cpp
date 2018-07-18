#include "HtmlTagInput.h"

namespace webserver
{
	HtmlTagInput::HtmlTagInput(const std::string& type, const std::string& label, const std::string& name, const std::string& value)
	: HtmlTag()
	{
		if (!label.compare("") && type.compare("text"))
		{
			HtmlTag labelTag("label");
			labelTag.AddContent(label);
			AddChildTag(labelTag);
		}
		HtmlTag inputTag("input");
		inputTag.AddAttribute("type", type);
		inputTag.AddContent(value);
		AddChildTag(inputTag);
	}
};

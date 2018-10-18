#include "webserver/HtmlTagInput.h"

namespace webserver
{
	HtmlTagInput::HtmlTagInput(const std::string& type, const std::string& name, const std::string& value)
	:	HtmlTag("input")
	{
		AddAttribute("type", type);
		AddAttribute("name", name);
		AddAttribute("id", name);
		if (value.size() > 0)
		{
			AddAttribute("value", value);
		}
	}
};

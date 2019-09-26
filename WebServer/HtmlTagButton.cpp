#include "WebServer/HtmlTagButton.h"
#include "WebServer/HtmlTagInput.h"

namespace WebServer
{
	HtmlTagButton::HtmlTagButton(const std::string& value, const std::string& command)
	:	commandID("b_" + command)
	{
		HtmlTag buttonTag("button");
		buttonTag.AddAttribute("name", commandID);
		buttonTag.AddAttribute("id", commandID);
		buttonTag.AddClass("button");
		buttonTag.AddContent(value);
		AddChildTag(buttonTag);
	}
};

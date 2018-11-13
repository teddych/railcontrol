#include "webserver/HtmlTagButton.h"
#include "webserver/HtmlTagInput.h"

namespace webserver
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

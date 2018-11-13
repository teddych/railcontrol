#include "webserver/HtmlTagButton.h"
#include "webserver/HtmlTagInput.h"

namespace webserver
{
	std::atomic<unsigned int> HtmlTagButton::buttonID(0);

	HtmlTagButton::HtmlTagButton(const std::string& value, const std::string& command)
	:	commandID("b_" + std::to_string(++buttonID) + "_" + command)
	{
		HtmlTag buttonTag("button");
		buttonTag.AddAttribute("name", commandID);
		buttonTag.AddAttribute("id", commandID);
		buttonTag.AddAttribute("class", "button");
		buttonTag.AddContent(value);
		AddChildTag(buttonTag);
	}
};

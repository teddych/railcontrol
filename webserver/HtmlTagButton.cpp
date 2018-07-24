#include "HtmlTagButton.h"
#include "HtmlTagInput.h"

namespace webserver
{
	std::atomic<unsigned int> HtmlTagButton::buttonID(0);

	HtmlTagButton::HtmlTagButton(const std::string& value, const std::string& command)
	:	commandID("b_" + std::to_string(++buttonID) + "_" + command)
	{
		HtmlTagInput inputTag("submit", commandID);
		inputTag.AddAttribute("class", "button");
		inputTag.AddAttribute("value", value);
		AddChildTag(inputTag);
	}
};

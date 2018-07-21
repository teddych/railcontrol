#include "HtmlTagButton.h"
#include "HtmlTagInput.h"

namespace webserver
{
	std::atomic<unsigned int> HtmlTagButton::buttonID(0);

	HtmlTagButton::HtmlTagButton(const std::string& value, const std::string& command, const std::string& javascript)
	:	commandID("b_" + std::to_string(++buttonID) + "_" + command),
		inputTag("submit", commandID),
	 	javascriptTag("script")
	{
		inputTag.AddAttribute("class", "button");
		inputTag.AddAttribute("value", value);

		if (javascript.size() > 0)
		{
			javascriptTag.AddAttribute("type", "application/javascript");
			javascriptTag.AddContent(javascript);
		}
	}

	std::ostream& operator<<(std::ostream& stream, const HtmlTagButton& tag)
	{
		stream << tag.inputTag;
		if (tag.javascriptTag.ContentSize() > 0)
		{
			stream << tag.javascriptTag;
		}
		return stream;
	}
};

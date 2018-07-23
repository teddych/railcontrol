#include "HtmlTagButton.h"
#include "HtmlTagInput.h"

namespace webserver
{
	std::atomic<unsigned int> HtmlTagButton::buttonID(0);

	HtmlTagButton::HtmlTagButton(const std::string& value, const std::string& command, const std::string& javascript)
	:	commandID("b_" + std::to_string(++buttonID) + "_" + command)
	{
		HtmlTagInput inputTag("submit", commandID);
		inputTag.AddAttribute("class", "button");
		inputTag.AddAttribute("value", value);
		AddChildTag(inputTag);
	}

	void HtmlTagButton::AddJavaScript(const std::string& content)
	{
	 	HtmlTag javascriptTag("script");
		javascriptTag.AddAttribute("type", "application/javascript");
		javascriptTag.AddContent(content);
		AddChildTag(javascriptTag);
	}

	void HtmlTagButton::AddAttribute(const std::string& name, const std::string& value)
	{
		if (childTags.size() >= 1)
		{
			childTags[0].AddAttribute(name, value);
			return;
		}
		HtmlTag::AddAttribute(name, value);
	}

	void HtmlTagButton::AddChildTag(const HtmlTag& child)
	{
		if (childTags.size() >= 1)
		{
			childTags[0].AddChildTag(child);
			return;
		}
		HtmlTag::AddChildTag(child);
	}

	void HtmlTagButton::AddContent(const std::string& content)
	{
		if (childTags.size() >= 1)
		{
			childTags[0].AddContent(content);
			return;
		}
		HtmlTag::AddContent(content);
	}
};

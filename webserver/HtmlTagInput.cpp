#include "HtmlTagInput.h"

namespace webserver
{
	HtmlTagInput::HtmlTagInput(const std::string& type, const std::string& name, const std::string& value, const std::string& label)
	:	HtmlTag()
	{
		if (label.size() > 0)
		{
			HtmlTag labelTag("label");
			labelTag.AddContent(label);
			labelTag.AddAttribute("for", name);
			AddChildTag(labelTag);
		}
		HtmlTag inputTag("input");
		inputTag.AddAttribute("type", type);
		inputTag.AddAttribute("name", name);
		inputTag.AddAttribute("id", name);
		if (value.size() > 0)
		{
			inputTag.AddAttribute("value", value);
		}
		AddChildTag(inputTag);
	}

	void HtmlTagInput::AddAttribute(const std::string& name, const std::string& value)
	{
		if (childTags.size() == 1)
		{
			childTags[0].AddAttribute(name, value);
			return;
		}
		if (childTags.size() >= 2)
		{
			childTags[1].AddAttribute(name, value);
			return;
		}
		HtmlTag::AddAttribute(name, value);
	}

	void HtmlTagInput::AddChildTag(const HtmlTag& child)
	{
		if (childTags.size() == 1)
		{
			childTags[0].AddChildTag(child);
			return;
		}
		if (childTags.size() >= 2)
		{
			childTags[1].AddChildTag(child);
			return;
		}
		HtmlTag::AddChildTag(child);
	}

	void HtmlTagInput::AddContent(const std::string& content)
	{
		if (childTags.size() == 1)
		{
			childTags[0].AddContent(content);
			return;
		}
		if (childTags.size() >= 2)
		{
			childTags[1].AddContent(content);
			return;
		}
		HtmlTag::AddContent(content);
	}
};

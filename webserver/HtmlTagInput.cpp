#include "HtmlTagInput.h"

namespace webserver
{
	HtmlTagInput::HtmlTagInput(const std::string& type, const std::string& name, const std::string& value, const std::string& label)
	:	HtmlTag(),
		labelTag("label"),
		inputTag("input")
	{
		if (label.size() > 0 && type.compare("text") == 0)
		{
			labelTag.AddContent(label);
			labelTag.AddAttribute("for", name);
		}
		inputTag.AddAttribute("type", type);
		inputTag.AddAttribute("name", name);
		inputTag.AddAttribute("id", name);
		if (value.size() > 0)
		{
			inputTag.AddContent(value);
		}
	}

	std::ostream& operator<<(std::ostream& stream, const HtmlTagInput& tag)
	{
		if (tag.labelTag.ContentSize() > 0)
		{
			stream << tag.labelTag;
		}
		stream << tag.inputTag;
		return stream;
	}
};

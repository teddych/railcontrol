#include "HtmlTag.h"

namespace webserver
{
	void HtmlTag::AddAttribute(const std::string& name, const std::string& value)
	{
		if (name.size() == 0)
		{
			return;
		}
		this->attributes[name] = value;
	}

	void HtmlTag::AddChildTag(const HtmlTag& child)
	{
		this->childTags.push_back(child);
	}

	void HtmlTag::AddContent(const std::string& content)
	{
		if (name.size() == 0)
		{
			this->content += content;
			return;
		}
		HtmlTag child;
		child.AddContent(content);
		AddChildTag(child);
	}

	std::ostream& operator<<(std::ostream& stream, const HtmlTag& tag)
	{
		if (tag.name.size() > 0)
		{
			stream << "<" << tag.name;
			for (auto attribute : tag.attributes)
			{
				stream << " " << attribute.first << "=" << "\"" << attribute.second << "\"";
			}

			if (tag.childTags.size() == 0 && tag.content.size() == 0)
			{
				stream << "/>";
				return stream;
			}
			stream << ">";
		}

		for (auto child : tag.childTags)
		{
			stream << child;
		}

		stream << tag.content;

		if (tag.name.size() > 0)
		{
			stream << "</" << tag.name << ">";
		}
		return stream;
	}
};

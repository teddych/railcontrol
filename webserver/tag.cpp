#include "webserver/tag.h"

namespace webserver
{
	void Tag::AddAttribute(std::string name, std::string value)
	{
		this->attributes[name] = value;
	}

	void Tag::AddChildTag(Tag child)
	{
		this->childTags.push_back(child);
	}

	void Tag::AddContent(std::string content)
	{
		if (name.size() == 0)
		{
			this->content = content;
			return;
		}
		Tag child;
		child.AddContent(content);
		AddChildTag(child);
	}

	std::ostream& operator<<(std::ostream& stream, const Tag& tag)
	{
		if (tag.name.size() == 0)
		{
			stream << tag.content;
			return stream;
		}

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

		for (auto child : tag.childTags)
		{
			stream << child;
		}

		stream << tag.content;
		stream << "</" << tag.name << ">";
		return stream;
	}
};

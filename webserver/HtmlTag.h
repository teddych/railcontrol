#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace webserver
{
	class HtmlTag
	{

		private:
			std::string name;
			std::vector<HtmlTag> childTags;
			std::map<std::string, std::string> attributes;
			std::string content;

		public:
			HtmlTag() {}
			HtmlTag(const std::string& name) : name(name) {}
			~HtmlTag() {};
			virtual void AddAttribute(const std::string& name, const std::string& value = "");
			virtual void AddChildTag(const HtmlTag& child);
			virtual void AddContent(const std::string& content);
			virtual size_t ContentSize() const { return content.size(); }
			friend std::ostream& operator<<(std::ostream& stream, const HtmlTag& tag);
	};
}; // namespace webserver


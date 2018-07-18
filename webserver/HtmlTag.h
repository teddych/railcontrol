#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace webserver
{
	class HtmlTag
	{
		public:
			HtmlTag() {}
			HtmlTag(const std::string& name) : name(name) {}
			~HtmlTag() {};
			void AddAttribute(const std::string& name, const std::string& value);
			void AddChildTag(const HtmlTag& child);
			void AddContent(const std::string& content);
			friend std::ostream& operator<<(std::ostream& stream, const HtmlTag& tag);

		private:
			std::string name;
			std::vector<HtmlTag> childTags;
			std::map<std::string, std::string> attributes;
			std::string content;

	};
}; // namespace webserver


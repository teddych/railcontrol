#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace webserver
{
	class Tag
	{
		public:
			Tag() {}
			Tag(const std::string name) : name(name) {}
			~Tag() {};
			void AddAttribute(const std::string name, const std::string value);
			void AddChildTag(const Tag child);
			void AddContent(const std::string content);
			friend std::ostream& operator<<(std::ostream& stream, const Tag& tag);

		private:
			std::string name;
			std::vector<Tag> childTags;
			std::map<std::string, std::string> attributes;
			std::string content;

	};
}; // namespace webserver


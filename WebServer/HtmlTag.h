/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace WebServer
{
	class HtmlTag
	{

		protected:
			std::string name;
			std::vector<HtmlTag> childTags;
			std::map<std::string, std::string> attributes;
			std::vector<std::string> classes;
			std::string content;

		public:
			HtmlTag() {}
			HtmlTag(const std::string& name) : name(name) {}
			virtual ~HtmlTag() {};
			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value = "");
			virtual HtmlTag AddChildTag(const HtmlTag& child);
			virtual HtmlTag AddContent(const std::string& content);
			virtual HtmlTag AddClass(const std::string& _class);
			virtual size_t ContentSize() const { return content.size(); }
			operator std::string () const;
			friend std::ostream& operator<<(std::ostream& stream, const HtmlTag& tag);
	};
}; // namespace WebServer


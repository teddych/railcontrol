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

#include <atomic>
#include <map>
#include <string>

#include "WebServer/HtmlTag.h"

namespace WebServer
{
	class HtmlTagSelect : public HtmlTag
	{
		private:
			const std::string commandID;

		public:
			HtmlTagSelect(const std::string& name, const std::map<std::string,std::string>& options, const std::string& defaultValue = "");
			template<typename T> HtmlTagSelect(const std::string& name, const std::map<std::string,T>& options, const int defaultValue = 0)
			:	HtmlTag("select"),
			 	commandID("s_" + name)
			{
				AddAttribute("name", name);
				AddAttribute("id", commandID);

				for (auto option : options)
				{
					HtmlTag optionTag("option");
					optionTag.AddAttribute("value", std::to_string(option.second));
					optionTag.AddContent(option.first);
					if (option.second == defaultValue)
					{
						optionTag.AddAttribute("selected");
					}
					AddChildTag(optionTag);
				}
			}
	};
};


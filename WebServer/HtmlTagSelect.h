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


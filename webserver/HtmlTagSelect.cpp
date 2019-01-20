#include "webserver/HtmlTagSelect.h"

namespace webserver
{
	HtmlTagSelect::HtmlTagSelect(const std::string& name, const std::map<std::string,std::string>& options, const std::string& defaultValue)
	:	HtmlTag("select"),
	 	commandID("s_" + name)
	{
		AddAttribute("name", name);
		AddAttribute("id", commandID);

		for (auto option : options)
		{
			HtmlTag optionTag("option");
			optionTag.AddAttribute("value", option.first);
			optionTag.AddContent(option.second);
			if (option.first.compare(defaultValue) == 0)
			{
				optionTag.AddAttribute("selected");
			}
			AddChildTag(optionTag);
		}
	}
};

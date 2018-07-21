#include "HtmlTagSelect.h"

namespace webserver
{
	std::atomic<unsigned int> HtmlTagSelect::selectID(0);

	HtmlTagSelect::HtmlTagSelect(const std::string& name, const std::map<std::string,std::string>& options, const std::string& defaultValue)
	:	HtmlTag("select"),
	 	commandID("s_" + std::to_string(++selectID) + "_" + name)
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

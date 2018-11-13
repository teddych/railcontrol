#include <sstream>
#include <vector>

#include "util.h"
#include "webserver/HtmlTagButtonCommandToggle.h"

namespace webserver
{
	HtmlTagButtonCommandToggle::HtmlTagButtonCommandToggle(const std::string& value, const std::string& command, const bool on, const std::map<std::string,std::string>& arguments)
	:	HtmlTagButton(value, command)
	{
		if (on == true)
		{
			AddClass("button_on");
		}
		{
			AddClass("button_off");
		}

		std::vector<std::string> parts;
		str_split(command, "_", parts);
		std::stringstream ss;
		ss <<
			"$(function() {"
			" $('#" << commandID << "').on('click', function() {"
			"  var on = !document.querySelector('#" << commandID << "').classList.contains('button_on');"
			"  var theUrl = '/?cmd=" << parts[0] << "&on=' + on + '";

		for (auto argument : arguments) {

			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';"
			"  var xmlHttp = new XMLHttpRequest();"
			"  xmlHttp.open('GET', theUrl, true);"
			"  xmlHttp.send(null);"
			"  return false;"
			" })"
			"})";
		AddJavaScript(ss.str());
	}
};

#include <sstream>

#include "webserver/HtmlTagButtonCommand.h"

namespace webserver
{
	HtmlTagButtonCommand::HtmlTagButtonCommand(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments)
	:	HtmlTagButton(value, command)
	{
		std::stringstream ss;
		ss <<
			"$(function() {"
			" $('#" << commandID << "').on('click', function() {"
			"  var theUrl = '/?cmd=" << command;
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

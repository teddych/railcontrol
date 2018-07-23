#include <sstream>

#include "HtmlTagButtonCommand.h"

namespace webserver
{
	HtmlTagButtonCommand::HtmlTagButtonCommand(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments)
	:	HtmlTagButton(value, command)
	{
		std::stringstream ss;
		ss <<
			"$(function() {\n"
			" $('#" << commandID << "').on('click', function() {\n"
			"  var theUrl = '/?cmd=" << command;
		for (auto argument : arguments) {

			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';\n"
			"  var xmlHttp = new XMLHttpRequest();\n"
			"  xmlHttp.open('GET', theUrl, true);\n"
			"  xmlHttp.send(null);\n"
			"  return false;\n"
			" })\n"
			"})\n";
		AddJavaScript(ss.str());
	}
};

#include <sstream>

#include "HtmlTagButtonCommand.h"
#include "HtmlTagInput.h"

namespace webserver
{
	std::atomic<unsigned int> HtmlTagButtonCommand::buttonID(0);

	HtmlTagButtonCommand::HtmlTagButtonCommand(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments)
	:	commandID(std::to_string(++buttonID) + "_" + command),
		inputTag("submit", commandID),
	 	javascriptTag("script")
	{
		inputTag.AddAttribute("class", "button");
		inputTag.AddAttribute("value", value);

		javascriptTag.AddAttribute("type", "application/javascript");
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
		javascriptTag.AddContent(ss.str());
	}

	std::ostream& operator<<(std::ostream& stream, const HtmlTagButtonCommand& tag)
	{
		stream << tag.inputTag;
		stream << tag.javascriptTag;
		return stream;
	}
};

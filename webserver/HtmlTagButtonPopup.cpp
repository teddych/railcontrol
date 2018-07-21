#include <sstream>

#include "HtmlTagButtonPopup.h"
#include "HtmlTagInput.h"

namespace webserver
{
	HtmlTagButtonPopup::HtmlTagButtonPopup(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments)
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
			"  $('#popup').show();\n"
			"  $('#popup').load(theUrl);\n"
			" })\n"
			"})\n";
		javascriptTag.AddContent(ss.str());
	}
};

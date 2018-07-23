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
			"  var myUrl = '/?cmd=" << command;
		for (auto argument : arguments) {

			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';\n"
			"  $('#popup').show(300);\n"
			"  $('#popup').load(myUrl);\n"
			" })\n"
			"})\n";
		AddJavaScript(ss.str());
	}
};

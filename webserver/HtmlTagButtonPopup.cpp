#include <sstream>

#include "webserver/HtmlTagButtonPopup.h"
#include "webserver/HtmlTagInput.h"

namespace webserver
{
	HtmlTagButtonPopup::HtmlTagButtonPopup(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments)
	:	HtmlTagButton(value, command)
	{
		std::stringstream ss;
		ss <<
			"$(function() {"
			" $('#" << commandID << "').on('click', function() {"
			"  var myUrl = '/?cmd=" << command;
		for (auto argument : arguments) {

			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';"
			"  $('#popup').show(300);"
			"  $('#popup').load(myUrl);"
			" })"
			"})";
		AddJavaScript(ss.str());
	}
};

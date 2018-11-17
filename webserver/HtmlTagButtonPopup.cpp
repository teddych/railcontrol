#include <sstream>
#include <vector>

#include "util.h"
#include "webserver/HtmlTagButtonPopup.h"
#include "webserver/HtmlTagInput.h"

namespace webserver
{
	HtmlTagButtonPopup::HtmlTagButtonPopup(const std::string& value, const std::string& command, const std::map<std::string,std::string>& arguments)
	:	HtmlTagButton(value, command)
	{
		std::vector<std::string> parts;
		str_split(command, "_", parts);
		std::stringstream ss;
		ss <<
			"$(function() {"
			" $('#" << commandID << "').on('click', function() {"
			"  var myUrl = '/?cmd=" << parts[0];
		for (auto argument : arguments) {

			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';"
			"  loadPopup(myUrl);"
			"  return false;"
			" })"
			"})";
		AddJavaScript(ss.str());
	}
};

#include <sstream>

#include "webserver/HtmlTagButtonCancel.h"

namespace webserver
{
	HtmlTagButtonCancel::HtmlTagButtonCancel()
	:	HtmlTagButton(HtmlTag("span").AddClass("symbola").AddContent("&#x2718;"), "popup_cancel")
	{
		std::stringstream s;
		s <<
			"$(function() {"
			" $('#" << commandID << "').on('click', function() {"
			"  $('#popup').hide(300);"
			"  return false;"
			" })"
			"})";
		AddJavaScript(s.str());
	}
};

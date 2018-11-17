#include <sstream>

#include "webserver/HtmlTagButtonCancel.h"

namespace webserver
{
	HtmlTagButtonCancel::HtmlTagButtonCancel()
	:	HtmlTagButton("Cancel", "popup_cancel")
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

#include <sstream>

#include "HtmlTagButtonCancel.h"

namespace webserver
{
	HtmlTagButtonCancel::HtmlTagButtonCancel()
	:	HtmlTagButton("Cancel", "popup_cancel")
	{
		std::stringstream s;
		s <<
			"$(function() {\n"
			" $('#" << commandID << "').on('click', function() {\n"
			"  $('#popup').hide(300);\n"
			" })\n"
			"})\n";
		AddJavaScript(s.str());
	}
};

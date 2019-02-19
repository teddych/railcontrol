#include <sstream>

#include "webserver/HtmlTagButtonCancel.h"

namespace webserver
{
	HtmlTagButtonCancel::HtmlTagButtonCancel()
	:	HtmlTagButton(HtmlTag("span").AddClass("symbola").AddContent("&#x2718;"), "popup_cancel")
	{
		std::stringstream ss;
		ss <<
			"$('#popup').hide(300);"
			"return false;";
		AddAttribute("onclick", ss.str());
	}
};

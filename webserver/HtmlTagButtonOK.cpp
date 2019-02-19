#include <sstream>

#include "webserver/HtmlTagButtonOK.h"

namespace webserver
{
	HtmlTagButtonOK::HtmlTagButtonOK()
	:	HtmlTagButton(HtmlTag("span").AddClass("symbola").AddContent("&#x2714;"), "popup_ok")
	{
		AddAttribute("onclick", "submitEditForm(); return false;");
	}
};

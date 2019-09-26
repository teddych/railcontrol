#include <sstream>

#include "WebServer/HtmlTagButtonOK.h"

namespace WebServer
{
	HtmlTagButtonOK::HtmlTagButtonOK()
	:	HtmlTagButton(HtmlTag("span").AddClass("symbola").AddContent("&#x2714;"), "popup_ok")
	{
		AddAttribute("onclick", "submitEditForm(); return false;");
	}
};

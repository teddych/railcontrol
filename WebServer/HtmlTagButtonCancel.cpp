#include <sstream>

#include "WebServer/HtmlTagButtonCancel.h"

namespace WebServer
{
	HtmlTagButtonCancel::HtmlTagButtonCancel()
	:	HtmlTagButton(HtmlTag("span").AddClass("symbola").AddContent("&#x2718;"), "popup_cancel")
	{
		AddAttribute("onclick", "document.getElementById('popup').style.display = 'none'; return false;");
	}
};

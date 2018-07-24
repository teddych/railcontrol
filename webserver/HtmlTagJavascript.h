#pragma once

#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagJavascript : public HtmlTag
	{
		public:
			HtmlTagJavascript(const std::string& javascript)
			: HtmlTag("script")
			{
				AddAttribute("type", "application/javascript");
				AddContent(javascript);
			};
	};
};


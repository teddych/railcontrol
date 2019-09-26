#pragma once

#include <string>

#include "WebServer/HtmlTag.h"

namespace WebServer
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


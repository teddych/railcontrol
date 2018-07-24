#pragma once

#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagLabel : public HtmlTag
	{
		public:
			HtmlTagLabel(const std::string& label, const std::string& reference)
			: HtmlTag("label")
			{
				AddContent(label);
				AddAttribute("for", reference);
			};
	};
};


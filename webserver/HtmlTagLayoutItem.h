#pragma once

#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagLayoutItem : public HtmlTag
	{
		public:
			HtmlTagLayoutItem() {}
			HtmlTagLayoutItem(const std::string& name) : HtmlTag(name) {}
			~HtmlTagLayoutItem() {};

			const unsigned char EdgeLength = 35;
			const std::string EdgeLengthString = std::to_string(EdgeLength);
	};
}; // namespace webserver


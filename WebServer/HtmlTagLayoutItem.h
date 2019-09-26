#pragma once

#include <string>

#include "WebServer/HtmlTag.h"

namespace WebServer
{
	class HtmlTagLayoutItem : public HtmlTag
	{
		public:
			HtmlTagLayoutItem() {}
			HtmlTagLayoutItem(const std::string& name) : HtmlTag(name) {}
			virtual ~HtmlTagLayoutItem() {};

			const unsigned char EdgeLength = 36;
			const std::string EdgeLengthString = std::to_string(EdgeLength);
	};
}; // namespace WebServer


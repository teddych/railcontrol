#pragma once

#include <string>

#include "WebServer/HtmlTag.h"

namespace WebServer
{
	class HtmlTagInputInteger : public HtmlTag
	{
		public:
			HtmlTagInputInteger(const std::string& name, const int value, const int min, const int max);
	};
};


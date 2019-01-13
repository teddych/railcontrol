#pragma once

#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagInputInteger : public HtmlTag
	{
		public:
			HtmlTagInputInteger(const std::string& name, const int value, const int min, const int max);
	};
};


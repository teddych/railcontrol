#pragma once

#include <ostream>
#include <string>

#include "HtmlResponse.h"

namespace webserver
{
	class HtmlResponseNotImplemented : public HtmlResponse
	{
		public:
			HtmlResponseNotImplemented(const std::string& method);
			~HtmlResponseNotImplemented() {};
	};
}; // namespace webserver


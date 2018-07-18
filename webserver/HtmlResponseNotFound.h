#pragma once

#include <ostream>
#include <string>

#include "HtmlResponse.h"

namespace webserver
{
	class HtmlResponseNotFound : public HtmlResponse
	{
		public:
			HtmlResponseNotFound(const std::string& file);
			~HtmlResponseNotFound() {};
	};
}; // namespace webserver


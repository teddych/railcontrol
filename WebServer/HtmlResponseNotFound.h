#pragma once

#include <ostream>
#include <string>

#include "WebServer/HtmlResponse.h"

namespace WebServer
{
	class HtmlResponseNotFound : public HtmlResponse
	{
		public:
			HtmlResponseNotFound(const std::string& file);
			~HtmlResponseNotFound() {};
	};
}; // namespace WebServer


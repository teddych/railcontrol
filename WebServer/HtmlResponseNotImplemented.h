#pragma once

#include <ostream>
#include <string>

#include "WebServer/HtmlResponse.h"

namespace WebServer
{
	class HtmlResponseNotImplemented : public HtmlResponse
	{
		public:
			HtmlResponseNotImplemented(const std::string& method);
			~HtmlResponseNotImplemented() {};
	};
}; // namespace WebServer


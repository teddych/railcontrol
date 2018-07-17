#pragma once

#include <ostream>
#include <string>

#include "Response.h"

namespace webserver
{
	class ResponseNotFound : public Response
	{
		public:
			ResponseNotFound(std::string& file);
			~ResponseNotFound() {};
	};
}; // namespace webserver


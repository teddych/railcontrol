#pragma once

#include <ostream>
#include <string>

#include "HtmlResponse.h"

namespace webserver
{
	class HtmlFullResponse : public HtmlResponse
	{
		public:
			HtmlFullResponse(const responseCode_t responseCode);
			HtmlFullResponse(const std::string& title, const HtmlTag body);
			HtmlFullResponse(const responseCode_t responseCode, const std::string& title, const HtmlTag body);
			~HtmlFullResponse() {};
			operator std::string();

			friend std::ostream& operator<<(std::ostream& stream, const HtmlFullResponse& response);

		private:
			std::string title;
	};
}; // namespace webserver


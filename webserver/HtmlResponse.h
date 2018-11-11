#pragma once

#include <ostream>
#include <string>

#include "Response.h"

namespace webserver
{
	class HtmlResponse : public Response
	{
		public:
			HtmlResponse(const responseCode_t responseCode)
			:	HtmlResponse(responseCode, std::to_string(responseCode) + " " + HtmlResponse::responseTexts.at(responseCode), HtmlTag("body"))
			{}

			HtmlResponse(const HtmlTag body)
			:	HtmlResponse("", body)
			{}

			HtmlResponse(const std::string& title, const HtmlTag body)
			:	HtmlResponse(Response::OK, title, body)
			{}

			HtmlResponse(const responseCode_t responseCode, const std::string& title, const HtmlTag body);
			~HtmlResponse() {};
			void AddAttribute(const std::string name, const std::string value);
			void AddChildTag(HtmlTag content);
			operator std::string();

			friend std::ostream& operator<<(std::ostream& stream, const HtmlResponse& response);

		protected:
			std::string title;
	};
}; // namespace webserver


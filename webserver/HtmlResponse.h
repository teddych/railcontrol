#pragma once

#include <ostream>
#include <string>

#include "HtmlTag.h"

namespace webserver
{
	class HtmlResponse
	{
		public:
			enum responseCode_t : unsigned short
			{
				OK = 200,
				NotFound = 404
			};
			HtmlResponse(const responseCode_t responseCode) : responseCode(responseCode), content(HtmlTag("body")) {}
			HtmlResponse(const responseCode_t responseCode, const HtmlTag tag) : responseCode(responseCode), content(tag) {}
			~HtmlResponse() {};
			void AddAttribute(const std::string name, const std::string value);
			void AddChildTag(HtmlTag content);

			friend std::ostream& operator<<(std::ostream& stream, const HtmlResponse& response);

		protected:
			responseCode_t responseCode;
			HtmlTag content;
			typedef std::map<HtmlResponse::responseCode_t,std::string> responseCodeMap;
			static const responseCodeMap responseTexts;
	};
}; // namespace webserver


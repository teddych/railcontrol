#pragma once

#include <ostream>
#include <string>

#include "webserver/tag.h"

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
			HtmlResponse(responseCode_t responseCode) : responseCode(responseCode), content(Tag("body")) {}
			HtmlResponse(responseCode_t responseCode, Tag tag) : responseCode(responseCode), content(tag) {}
			~HtmlResponse() {};
			void AddAttribute(const std::string name, const std::string value);
			void AddChildTag(Tag content);

			friend std::ostream& operator<<(std::ostream& stream, const HtmlResponse& response);

		protected:
			responseCode_t responseCode;
			Tag content;
			typedef std::map<HtmlResponse::responseCode_t,std::string> responseCodeMap;
			static const responseCodeMap responseTexts;
	};
}; // namespace webserver


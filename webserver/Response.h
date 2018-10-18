#pragma once

#include <map>
#include <ostream>
#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class Response
	{
		public:
			enum responseCode_t : unsigned short
			{
				OK = 200,
				NotFound = 404
			};
			Response(const responseCode_t responseCode) : responseCode(responseCode) {}
			Response(const responseCode_t responseCode, const HtmlTag& content) : responseCode(responseCode), content(content) {}
			Response(const responseCode_t responseCode, const std::string& content) : responseCode(responseCode) { this->content.AddContent(content); }
			~Response() {};
			void AddHeader(const std::string& key, const std::string& value);
			std::string ToString();

			friend std::ostream& operator<<(std::ostream& stream, const Response& response);

			responseCode_t responseCode;

			typedef std::map<Response::responseCode_t,std::string> responseCodeMap;
			static const responseCodeMap responseTexts;

			std::map<const std::string,std::string> headers;
			HtmlTag content;
	};
}; // namespace webserver


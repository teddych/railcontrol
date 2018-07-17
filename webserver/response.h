#pragma once

#include <ostream>
#include <string>

#include "webserver/tag.h"

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
			Response(responseCode_t responseCode, Tag& tag) : responseCode(responseCode), tag(tag) {}
			~Response() {};

			friend std::ostream& operator<<(std::ostream& stream, const Response& response);

		private:
			responseCode_t responseCode;
			Tag& tag;
			typedef std::map<Response::responseCode_t,std::string> responseCodeMap;
			static const responseCodeMap responseTexts;
	};
}; // namespace webserver


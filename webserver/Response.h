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
			Response(responseCode_t responseCode, Tag tag) : responseCode(responseCode), content(tag) {}
			~Response() {};
			void AddChildTag(Tag content);

			friend std::ostream& operator<<(std::ostream& stream, const Response& response);

		protected:
			responseCode_t responseCode;
			Tag content;
			typedef std::map<Response::responseCode_t,std::string> responseCodeMap;
			static const responseCodeMap responseTexts;
	};
}; // namespace webserver


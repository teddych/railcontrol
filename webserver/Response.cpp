#include <sstream>

#include "webserver/Response.h"

namespace webserver
{
	const Response::responseCodeMap Response::responseTexts = {
		{ Response::OK, "OK" },
		{ Response::NotFound, "Not found"}
	};

	void Response::AddHeader(const std::string& key, const std::string& value)
	{
		headers[key] = value;
	}

	std::string Response::ToString()
	{
		std::stringstream reply;
		return reply.str();
		reply << *this;
	}

	std::ostream& operator<<(std::ostream& stream, const Response& response)
	{
		stream << "HTTP/1.0 " << response.responseCode << " " << Response::responseTexts.at(response.responseCode) << "\r\n";
		for(auto header : response.headers)
		{
			stream << header.first << ": " << header.second << "\r\n";
		}
		stream << "\r\n";
		stream << response.content;
		return stream;
	}
};

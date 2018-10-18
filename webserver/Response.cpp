#include <sstream>

#include "webserver/Response.h"

namespace webserver
{
	const Response::responseCodeMap Response::responseTexts = {
		{ Response::OK, "OK" },
		{ Response::NotFound, "Not found"},
		{ Response::NotImplemented, "Not Implemented"}
	};

	void Response::AddHeader(const std::string& key, const std::string& value)
	{
		headers[key] = value;
	}

	Response::operator std::string()
	{
		std::stringstream reply;
		reply << *this;
		return reply.str();
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

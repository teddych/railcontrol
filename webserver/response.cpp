#include "webserver/response.h"

namespace webserver
{
	const Response::responseCodeMap Response::responseTexts = {
		{ Response::OK, "OK" },
		{ Response::NotFound, "Not found"}
	};

	std::ostream& operator<<(std::ostream& stream, const Response& response)
	{
		stream << "HTTP/1.0 " << response.responseCode << " " << Response::responseTexts.at(response.responseCode) << "\r\n\r\n";
		stream << "<!DOCTYPE html>";
		stream << response.tag;
		return stream;
	}
};

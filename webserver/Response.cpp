#include "Response.h"

namespace webserver
{
	const Response::responseCodeMap Response::responseTexts = {
		{ Response::OK, "OK" },
		{ Response::NotFound, "Not found"}
	};

	void Response::AddChildTag(Tag content)
	{
		this->content.AddChildTag(content);
	}


	std::ostream& operator<<(std::ostream& stream, const Response& response)
	{
		stream << "HTTP/1.0 " << response.responseCode << " " << Response::responseTexts.at(response.responseCode) << "\r\n\r\n";
		stream << "<!DOCTYPE html>";

		Tag head("head");
		Tag title("title");
		title.AddContent(std::to_string(response.responseCode));
		title.AddContent(" ");
		title.AddContent(Response::responseTexts.at(response.responseCode));
		head.AddChildTag(title);
		Tag body("body");
		body.AddChildTag(response.content);
		Tag html("html");
		html.AddChildTag(head);
		html.AddChildTag(body);

		stream << html;
		return stream;
	}
};

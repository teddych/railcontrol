#include <webserver/HtmlResponse.h>

namespace webserver
{
	void HtmlResponse::AddAttribute(const std::string name, const std::string value)
	{
		this->content.AddAttribute(name, value);
	}

	void HtmlResponse::AddChildTag(HtmlTag content)
	{
		this->content.AddChildTag(content);
	}

	std::ostream& operator<<(std::ostream& stream, const HtmlResponse& response)
	{
		stream << "HTTP/1.0 " << response.responseCode << " " << HtmlResponse::responseTexts.at(response.responseCode) << "\r\n";
		for(auto header : response.headers)
		{
			stream << header.first << ": " << header.second << "\r\n";
		}
		stream << "\r\n";
		stream << "<!DOCTYPE html>";

		HtmlTag title("title");
		title.AddContent(std::to_string(response.responseCode));
		title.AddContent(" ");
		title.AddContent(HtmlResponse::responseTexts.at(response.responseCode));

		HtmlTag head("head");
		head.AddChildTag(title);

		stream << HtmlTag("html").AddChildTag(head).AddChildTag(response.content);
		return stream;
	}
};

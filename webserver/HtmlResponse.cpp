#include <sstream>

#include "webserver/HtmlResponse.h"

namespace webserver
{
	HtmlResponse::HtmlResponse(const responseCode_t responseCode)
	:	HtmlResponse(responseCode, std::to_string(responseCode) + " " + HtmlResponse::responseTexts.at(responseCode), HtmlTag("body"))
	{}

	HtmlResponse::HtmlResponse(const std::string& title, const HtmlTag body)
	:	HtmlResponse(Response::OK, title, body)
	{}

	HtmlResponse::HtmlResponse(const responseCode_t responseCode, const std::string& title, const HtmlTag body)
	:	Response(responseCode, body),
	 	title(title)
	{
		AddHeader("Cache-Control", "no-cache, must-revalidate");
		AddHeader("Pragma", "no-cache");
		AddHeader("Expires", "Sun, 12 Feb 2016 00:00:00 GMT");
		AddHeader("Content-Type", "text/html; charset=utf-8");
	}

	void HtmlResponse::AddAttribute(const std::string name, const std::string value)
	{
		this->content.AddAttribute(name, value);
	}

	void HtmlResponse::AddChildTag(HtmlTag content)
	{
		this->content.AddChildTag(content);
	}

	HtmlResponse::operator std::string()
	{
		std::stringstream reply;
		reply << *this;
		return reply.str();
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

		HtmlTag head("head");
		head.AddChildTag(HtmlTag("title").AddContent(response.title));
		head.AddChildTag(HtmlTag("link").AddAttribute("rel", "stylesheet").AddAttribute("type", "text/css").AddAttribute("href", "/style.css"));
		head.AddChildTag(HtmlTag("script").AddAttribute("type", "application/javascript").AddAttribute("src", "/jquery-3.1.1.min.js"));
		head.AddChildTag(HtmlTag("script").AddAttribute("type", "application/javascript").AddAttribute("src", "/javascript.js"));
		head.AddChildTag(HtmlTag("meta").AddAttribute("name", "viewport").AddAttribute("content", "width=device-width, initial-scale=1.0"));
		head.AddChildTag(HtmlTag("meta").AddAttribute("name", "robots").AddAttribute("content", "noindex,nofollow"));

		stream << HtmlTag("html").AddChildTag(head).AddChildTag(response.content);
		return stream;
	}
};

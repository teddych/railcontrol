#include "WebServer/HtmlResponseNotFound.h"

namespace WebServer
{
	using std::string;

	HtmlResponseNotFound::HtmlResponseNotFound(const string& file)
	: HtmlResponse(HtmlResponse::NotFound)
	{
		content.AddChildTag(HtmlTag("h1").AddContent("File not found"));
		content.AddChildTag(HtmlTag("p").AddContent("File ").AddContent(file).AddContent(" not found"));
	}
};

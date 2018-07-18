#include <webserver/HtmlResponseNotFound.h>

namespace webserver
{
	using std::string;

	HtmlResponseNotFound::HtmlResponseNotFound(const string& file)
	: HtmlResponse(HtmlResponse::NotFound)
	{
		HtmlTag title("h1");
		title.AddContent("File not found");
		HtmlTag p("p");
		p.AddContent("File ");
		p.AddContent(file);
		p.AddContent(" not found");
		content.AddChildTag(title);
		content.AddChildTag(p);
	}
};

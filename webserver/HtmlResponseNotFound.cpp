#include <webserver/HtmlResponseNotFound.h>

namespace webserver
{
	using std::string;

	HtmlResponseNotFound::HtmlResponseNotFound(const string& file)
	: HtmlResponse(HtmlResponse::NotFound)
	{
		Tag title("h1");
		title.AddContent("File not found");
		Tag p("p");
		p.AddContent("File ");
		p.AddContent(file);
		p.AddContent(" not found");
		content.AddChildTag(title);
		content.AddChildTag(p);
	}
};

#include "webserver/ResponseNotFound.h"

namespace webserver
{
	using std::string;

	ResponseNotFound::ResponseNotFound(string& file)
	: Response(Response::NotFound, Tag("h1"))
	{
		content.AddContent("File ");
		content.AddContent(file);
		content.AddContent(" not found");
	}
};

#include <string>

#include "webserver/HtmlTagInput.h"
#include "webserver/HtmlTagInputInteger.h"

namespace webserver
{
	HtmlTagInputInteger::HtmlTagInputInteger(const std::string& name, const int value, const int min, const int max)
	{
		std::string minString = std::to_string(min);
		std::string maxString = std::to_string(max);
		HtmlTag data;
		HtmlTagInput input("text", name, std::to_string(value));
		input.AddClass("integer");
		input.AddAttribute("onchange", "checkIntegerValue('" + name + "', " + minString + ", " + maxString + ");");
		data.AddChildTag(input);

		HtmlTag plus("a");
		plus.AddAttribute("onclick", "incrementIntegerValue('" + name + "', " + maxString + ");");
		plus.AddContent(" + ");
		data.AddChildTag(plus);

		HtmlTag minus("a");
		minus.AddAttribute("onclick", "decrementIntegerValue('" + name + "', " + minString + ");");
		minus.AddContent(" - ");
		data.AddChildTag(minus);

		data.AddChildTag(HtmlTag("br"));
		AddChildTag(data);
	}
};

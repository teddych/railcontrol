#include <iomanip>
#include <sstream>
#include <string>

#include "Server/Web/HtmlTagInputDelay.h"

namespace Server { namespace Web
{
	namespace
	{
		std::string DelayToSecondsString(const int value)
		{
			std::stringstream stream;
			stream << std::fixed << std::setprecision(1) << static_cast<double>(value) / 10.0;
			return stream.str();
		}
	}

	HtmlTagInputDelay::HtmlTagInputDelay(const std::string& name, const int value, const int min, const int max)
	: HtmlTag("div")
	{
		const std::string minString = std::to_string(min);
		const std::string maxString = std::to_string(max);
		const std::string valueName = name + "_value";

		AddId("d_" + name);
		AddClass("div_integer");
		AddClass("div_delay");

		HtmlTag hidden("input");
		hidden.AddAttribute("type", "hidden");
		hidden.AddAttribute("name", name);
		hidden.AddAttribute("value", std::to_string(value));
		hidden.AddId(valueName);
		hidden.AddClass("hidden");
		AddChildTag(hidden);

		HtmlTag input("input");
		input.AddAttribute("type", "number");
		input.AddAttribute("min", DelayToSecondsString(min));
		input.AddAttribute("max", DelayToSecondsString(max));
		input.AddAttribute("step", "0.1");
		input.AddAttribute("value", DelayToSecondsString(value));
		input.AddId(name);
		input.AddClass("integer");
		input.AddClass("delay");
		input.AddAttribute("onfocus", "this.select();");
		input.AddAttribute("onclick", "this.select();");
		input.AddAttribute("oninput", "checkDelayValue('" + name + "', " + minString + ", " + maxString + ");");
		AddChildTag(input);
		AddChildTag(HtmlTag("span").AddClass("delay_unit").AddContent("s"));
	}
}} // namespace Server::Web
#include "HtmlTagInputSliderLocoSpeed.h"

namespace webserver
{
	HtmlTagInputSliderLocoSpeed::HtmlTagInputSliderLocoSpeed(const std::string& name, const std::string& cmd, const unsigned int min, const unsigned int max, const unsigned int value, const locoID_t locoID)
	: HtmlTagInputSlider(name, min, max, value)
	{
		std::string reference = "locospeed_" + std::to_string(locoID);
		AddAttribute("id", reference);
		AddAttribute("class", "slider");

		std::stringstream ss;
		ss <<"$(function() {\n"
			" $('#" << reference << "').on('change', function() {\n"
			"  var myUrl = '/?cmd=locospeed&loco=" << locoID << "&speed=' + document.getElementById('" << reference << "').value;\n"
			"  var xmlHttp = new XMLHttpRequest();\n"
			"  xmlHttp.open('GET', myUrl, true);\n"
			"  xmlHttp.send(null);\n"
			"  return false;\n"
			" })\n"
			"});\n";

		AddJavaScript(ss.str());
	};
};

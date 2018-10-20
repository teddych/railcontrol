#include "webserver/HtmlTagInputSliderLocoSpeed.h"

namespace webserver
{
	HtmlTagInputSliderLocoSpeed::HtmlTagInputSliderLocoSpeed(const std::string& name, const std::string& cmd, const unsigned int min, const unsigned int max, const unsigned int value, const locoID_t locoID)
	: HtmlTagInputSlider(name, min, max, value)
	{
		std::string reference = "locospeed_" + std::to_string(locoID);
		AddAttribute("id", reference);
		AddAttribute("class", "slider");

		std::stringstream ss;
		ss <<"$(function() {"
			" $('#" << reference << "').on('change', function() {"
			"  var myUrl = '/?cmd=locospeed&loco=" << locoID << "&speed=' + document.getElementById('" << reference << "').value;"
			"  var xmlHttp = new XMLHttpRequest();"
			"  xmlHttp.open('GET', myUrl, true);"
			"  xmlHttp.send(null);"
			"  return false;"
			" })"
			"});";

		AddJavaScript(ss.str());
	};
};

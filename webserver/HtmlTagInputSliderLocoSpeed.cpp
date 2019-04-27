#include "webserver/HtmlTagInputSliderLocoSpeed.h"

namespace webserver
{
	HtmlTagInputSliderLocoSpeed::HtmlTagInputSliderLocoSpeed(const std::string& name, const unsigned int min, const unsigned int max, const unsigned int value, const locoID_t locoID)
	: HtmlTagInputSlider(name, min, max, value)
	{
		std::string locoIdString = std::to_string(locoID);
		std::string reference = "locospeed_" + locoIdString;
		AddAttribute("id", reference);
		AddClass("slider");
		AddAttribute("onchange", "locoSpeedSliderChange(" + locoIdString + "); return false;");
	};
};

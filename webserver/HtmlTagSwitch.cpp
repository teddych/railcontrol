#include <sstream>

#include "webserver/HtmlTagSwitch.h"
#include "webserver/HtmlTagJavascript.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagSwitch::HtmlTagSwitch(const datamodel::Switch* mySwitch)
	{
		layoutPosition_t posX;
		layoutPosition_t posY;
		layoutPosition_t posZ;
		layoutItemSize_t w;
		layoutItemSize_t h;
		layoutRotation_t r;
		mySwitch->position(posX, posY, posZ, w, h, r);
		switchState_t state = mySwitch->GetState();
		switchType_t type = mySwitch->GetType();
		Init(mySwitch->objectID, mySwitch->name, posX, posY, posZ, mySwitch->Rotation(), state, type, mySwitch->address);
	}

	void HtmlTagSwitch::Init(const switchID_t switchID,
		const std::string& name,
		const layoutPosition_t posX,
		const layoutPosition_t posY,
		const layoutPosition_t posZ,
		const string rotation,
		const switchState_t state,
		const switchType_t type,
		const address_t address)
	{
		unsigned int layoutPosX = posX * 35;
		unsigned int layoutPosY = posY * 35;

		HtmlTag div1("div");
		string switchIdString = to_string(switchID);
		string id("sw_" + switchIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("switch_item");
		div1.AddClass(state == SwitchStateStraight ? "switch_straight" : "switch_turnout");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		if (type == SwitchTypeLeft)
		{
			div1.AddChildTag(HtmlTag("span").AddContent("<svg width=\"35\" height=\"35\" id=\"" + id + "_img\" style=\"transform:rotate(" + rotation + "deg);\"><polygon points=\"13,26 22,35 13,35\" fill=\"black\" /><polygon points=\"0,13 13,26 13,35 0,22\" fill=\"gray\" class=\"turnout\"/><polygon points=\"13,0 22,0 22,35 13,26\" fill=\"gray\" class=\"straight\"/></svg>"));
		}
		else
		{
			div1.AddChildTag(HtmlTag("span").AddContent("<svg width=\"35\" height=\"35\" id=\"" + id + "_img\" style=\"transform:rotate(" + rotation + "deg);\"><polygon points=\"22,26 22,35 13,35\" fill=\"black\" /><polygon points=\"22,26 35,13 35,22 22,35\" fill=\"gray\" class=\"turnout\"/><polygon points=\"13,0 22,0 22,26 13,35\" fill=\"gray\" class=\"straight\"/></svg>"));
		}
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(name + " (addr=" + to_string(address) + ")"));

		std::stringstream javascript;
		javascript << "$(function() {"
			" $('#" << id << "').on('click', function() { onClickSwitch(" << switchID << "); return false; });"
			" $('#" << id << "').on('contextmenu', function(event) { if (event.shiftKey) return true; event.preventDefault(); onContextSwitch(" << switchID << "); return false; });"
			"});"
			;
		div1.AddChildTag(HtmlTagJavascript(javascript.str()));
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=switchedit&switch=" + switchIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=switchaskdelete&switch=" + switchIdString + "');"))
			);
		AddChildTag(div2);
	}
};

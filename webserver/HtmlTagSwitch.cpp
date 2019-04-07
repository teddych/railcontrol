#include <sstream>

#include "webserver/HtmlTagSwitch.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagSwitch::HtmlTagSwitch(const datamodel::Switch* mySwitch)
	{
		switchState_t state = mySwitch->GetState();
		switchType_t type = mySwitch->GetType();

		unsigned int layoutPosX = mySwitch->posX * EdgeLength;
		unsigned int layoutPosY = mySwitch->posY * EdgeLength;

		HtmlTag div1("div");
		string switchIdString = to_string(mySwitch->objectID);
		string id("sw_" + switchIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("switch_item");
		div1.AddClass(state == SwitchStateStraight ? "switch_straight" : "switch_turnout");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		string image;
		if (type == SwitchTypeLeft)
		{
			image = "<svg width=\"" + EdgeLengthString + "\" height=\"" + EdgeLengthString + "\" id=\"" + id + "_img\" style=\"transform:rotate(" + datamodel::LayoutItem::Rotation(mySwitch->rotation) + "deg);\"><polygon points=\"13,26 22,35 13,35\" fill=\"black\" /><polygon points=\"0,13 13,26 13,35 0,22\" fill=\"gray\" class=\"turnout\"/><polygon points=\"13,0 22,0 22,35 13,26\" fill=\"gray\" class=\"straight\"/></svg>";
		}
		else
		{
			image = "<svg width=\"" + EdgeLengthString + "\" height=\"" + EdgeLengthString + "\" id=\"" + id + "_img\" style=\"transform:rotate(" + datamodel::LayoutItem::Rotation(mySwitch->rotation) + "deg);\"><polygon points=\"22,26 22,35 13,35\" fill=\"black\" /><polygon points=\"22,26 35,13 35,22 22,35\" fill=\"gray\" class=\"turnout\"/><polygon points=\"13,0 22,0 22,26 13,35\" fill=\"gray\" class=\"straight\"/></svg>";
		}
		div1.AddChildTag(HtmlTag().AddContent(image));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(mySwitch->Name() + " (addr=" + to_string(mySwitch->GetAddress()) + ")"));
		div1.AddAttribute("onclick", "return onClickSwitch(" + switchIdString + ");");
		div1.AddAttribute("oncontextmenu", "return onContextLayoutItem(event, '" + id + "');");
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent(mySwitch->Name()))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Release").AddAttribute("onClick", "fireRequestAndForget('/?cmd=switchrelease&switch=" + switchIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=switchedit&switch=" + switchIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=switchaskdelete&switch=" + switchIdString + "');"))
			);
		AddChildTag(div2);
	}
};

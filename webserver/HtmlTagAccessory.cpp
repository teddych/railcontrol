#include <sstream>

#include "webserver/HtmlTagAccessory.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagAccessory::HtmlTagAccessory(const datamodel::Accessory* accessory)
	{
		accessoryState_t state = accessory->GetState();

		unsigned int layoutPosX = accessory->posX * EdgeLength;
		unsigned int layoutPosY = accessory->posY * EdgeLength;

		HtmlTag div1("div");
		string accessoryIdString = to_string(accessory->objectID);
		string id("a_" + accessoryIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("accessory_item");
		div1.AddClass(state == AccessoryStateOn ? "accessory_on" : "accessory_off");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		div1.AddChildTag(HtmlTag("span").AddClass("symbola").AddContent("&#9209;"));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(accessory->name + " (addr=" + to_string(accessory->address) + ")"));
		div1.AddAttribute("onclick", "return onClickAccessory(" + accessoryIdString + ");");
		div1.AddAttribute("oncontextmenu", "return onContextLayoutItem(event, '" + id + "');");
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent(accessory->name))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Release").AddAttribute("onClick", "fireRequestAndForget('/?cmd=accessoryrelease&accessory=" + accessoryIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=accessoryedit&accessory=" + accessoryIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=accessoryaskdelete&accessory=" + accessoryIdString + "');"))
			);
		AddChildTag(div2);
	}
};

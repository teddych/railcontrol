#include <sstream>

#include "webserver/HtmlTagAccessory.h"
#include "webserver/HtmlTagJavascript.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagAccessory::HtmlTagAccessory(const datamodel::Accessory* accessory)
	{
		layoutPosition_t posX;
		layoutPosition_t posY;
		layoutPosition_t posZ;
		layoutItemSize_t w;
		layoutItemSize_t h;
		layoutRotation_t r;
		accessory->position(posX, posY, posZ, w, h, r);
		accessoryState_t state = accessory->GetState();
		Init(accessory->objectID, accessory->name, posX, posY, posZ, state, accessory->address);
	}

	void HtmlTagAccessory::Init(const accessoryID_t accessoryID,
		const std::string& name,
		const layoutPosition_t posX,
		const layoutPosition_t posY,
		const layoutPosition_t posZ,
		const accessoryState_t state,
		const address_t address)
	{
		unsigned int layoutPosX = posX * 35;
		unsigned int layoutPosY = posY * 35;

		HtmlTag div1("div");
		string accessoryIdString = to_string(accessoryID);
		string id("a_" + accessoryIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("accessory_item");
		div1.AddClass(state == AccessoryStateOn ? "accessory_on" : "accessory_off");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		div1.AddChildTag(HtmlTag("span").AddClass("symbola").AddContent("&#9209;"));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(name + " (addr=" + to_string(address) + ")"));

		std::stringstream javascript;
		javascript << "$(function() {"
			" $('#" << id << "').on('click', function() { onClickAccessory(" << accessoryID << "); return false; });"
			" $('#" << id << "').on('contextmenu', function(event) { if (event.shiftKey) return true; event.preventDefault(); onContextAccessory(" << accessoryID << "); return false; });"
			"});"
			;
		div1.AddChildTag(HtmlTagJavascript(javascript.str()).AddClass("layout_item_script"));
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=accessoryedit&accessory=" + accessoryIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=accessoryaskdelete&accessory=" + accessoryIdString + "');"))
			);
		AddChildTag(div2);
	}
};

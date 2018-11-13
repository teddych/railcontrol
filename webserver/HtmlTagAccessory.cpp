#include <sstream>

#include "webserver/HtmlTagAccessory.h"
#include "webserver/HtmlTagJavascript.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagAccessory::HtmlTagAccessory(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const accessoryState_t state, const address_t address)
	{
		unsigned int layoutPosX = posX * 35;
		unsigned int layoutPosY = posY * 35;

		HtmlTag div1("div");
		string accessoryIdString = to_string(accessoryID);
		string id("a_" + accessoryIdString);
		div1.AddAttribute("id", id);
		string classes("layout_item accessory_item");
		if (state == AccessoryStateOn)
		{
			classes += " accessory_on";
		}
		div1.AddAttribute("class", classes);
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		div1.AddContent("&#9209;");
		div1.AddChildTag(HtmlTag("span").AddAttribute("class", "tooltip").AddContent(name + " (addr=" + to_string(address) + ")"));

		std::stringstream javascript;
		javascript << "$(function() {"
			" $('#" << id << "').on('click', function() {"
			"  var element = document.getElementById('" << id << "');"
			"  var url = '/?cmd=accessorystate';"
			"  url += '&state=' + (element.classList.contains('accessory_on') ? 'off' : 'on');"
			"  url += '&accessory=" << accessoryID << "';"
			"  var xmlHttp = new XMLHttpRequest();"
			"  xmlHttp.open('GET', url, true);"
			"  xmlHttp.send(null);"
			"  return false;"
			" });"
			" $('#" << id << "').on('contextmenu', function(event) {"
			"  if (event.shiftKey) {"
			"   return true;"
			"  }"
			"  event.preventDefault();"
			"  hideAllContextMenus();"
			"  menu = document.getElementById('" << id << "_context');"
			"  if (menu) {"
			"   menu.style.display = 'block';"
			"  }"
			"  return false;"
			" });"
			"});"
			;
		div1.AddChildTag(HtmlTagJavascript(javascript.str()));
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddAttribute("class", "contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddAttribute("class", "contextentries")
			.AddChildTag(HtmlTag("li").AddAttribute("class", "contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=accessoryedit&accessory=" + accessoryIdString + "');"))
			);
		AddChildTag(div2);
	}
};

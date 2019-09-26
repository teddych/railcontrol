#include <sstream>

#include "datamodel/signal.h"
#include "WebServer/HtmlTagSignal.h"

using std::string;
using std::to_string;

namespace WebServer
{
	HtmlTagSignal::HtmlTagSignal(const datamodel::Signal* signal)
	{
		signalState_t state = signal->GetState();
		signalType_t type = signal->GetType();

		unsigned int layoutPosX = signal->GetPosX() * EdgeLength;
		unsigned int layoutPosY = signal->GetPosY() * EdgeLength;
		const string& signalName = signal->GetName();

		HtmlTag div1("div");
		string signalIdString = to_string(signal->GetID());
		string id("si_" + signalIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("signal_item");
		div1.AddClass(state == datamodel::Signal::SignalStateRed ? "signal_red" : "signal_green");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		string image;
		if (type == datamodel::Signal::SignalTypeSimple)
		{
			image = "<svg width=\"" + EdgeLengthString + "\" height=\"" + EdgeLengthString + "\" id=\"" + id + "_img\" style=\"transform:rotate(" + datamodel::LayoutItem::Rotation(signal->GetRotation()) + "deg);\"><polygon points=\"14,0 22,0 22,36 14,36\" fill=\"black\" /><polygon points=\"1,5 5,1 9,1 13,5 13,18 9,22 5,22 1,18\" fill=\"black\"/><polyline points=\"7,7 7,30\" style=\"stroke:black;stroke-width:2\"/><circle class=\"red\" cx=\"7\" cy=\"7\" r=\"4\" fill=\"darkgray\"/><circle class=\"green\" cx=\"7\" cy=\"16\" r=\"4\" fill=\"darkgray\"/></svg>";
		}
		div1.AddChildTag(HtmlTag().AddContent(image));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(signalName + " (addr=" + to_string(signal->GetAddress()) + ")"));
		div1.AddAttribute("onclick", "return onClickSignal(" + signalIdString + ");");
		div1.AddAttribute("oncontextmenu", "return onContextLayoutItem(event, '" + id + "');");
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent(signalName))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Release").AddAttribute("onClick", "fireRequestAndForget('/?cmd=signalrelease&signal=" + signalIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=signaledit&signal=" + signalIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=signalaskdelete&signal=" + signalIdString + "');"))
			);
		AddChildTag(div2);
	}
};

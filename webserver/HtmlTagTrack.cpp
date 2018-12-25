#include <sstream>

#include "webserver/HtmlTagTrack.h"
#include "webserver/HtmlTagJavascript.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagTrack::HtmlTagTrack(const datamodel::Track* track)
	{
		layoutPosition_t posX;
		layoutPosition_t posY;
		layoutPosition_t posZ;
		layoutItemSize_t w;
		layoutItemSize_t h;
		layoutRotation_t r;
		track->position(posX, posY, posZ, w, h, r);
		trackType_t type = track->Type();
		Init(track->objectID, track->name, posX, posY, posZ, track->Rotation(), type);
	}

	void HtmlTagTrack::Init(const trackID_t trackID,
		const std::string& name,
		const layoutPosition_t posX,
		const layoutPosition_t posY,
		const layoutPosition_t posZ,
		const string rotation,
		const trackType_t type)
	{
		unsigned int layoutPosX = posX * 35;
		unsigned int layoutPosY = posY * 35;

		HtmlTag div1("div");
		string trackIdString = to_string(trackID);
		string id("sw_" + trackIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("track_item");
		div1.AddClass(type == TrackTypeStraight ? "track_straight" : "track_turnout"); // FIXME
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		if (type == TrackTypeLeft)
		{
			div1.AddChildTag(HtmlTag("span").AddContent("<svg width=\"35\" height=\"35\" id=\"" + id + "_img\" style=\"transform:rotate(" + rotation + "deg);\"><polygon points=\"13,26 22,35 13,35\" fill=\"black\" /><polygon points=\"0,13 13,26 13,35 0,22\" fill=\"gray\" class=\"turnout\"/><polygon points=\"13,0 22,0 22,35 13,26\" fill=\"gray\" class=\"straight\"/></svg>")); // FIXME
		}
		else
		{
			div1.AddChildTag(HtmlTag("span").AddContent("<svg width=\"35\" height=\"35\" id=\"" + id + "_img\" style=\"transform:rotate(" + rotation + "deg);\"><polygon points=\"22,26 22,35 13,35\" fill=\"black\" /><polygon points=\"22,26 35,13 35,22 22,35\" fill=\"gray\" class=\"turnout\"/><polygon points=\"13,0 22,0 22,26 13,35\" fill=\"gray\" class=\"straight\"/></svg>")); // FIXME
		}
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(name));

		std::stringstream javascript;
		javascript << "$(function() {"
			" $('#" << id << "').on('click', function() { onClickTrack(" << trackID << "); return false; });"
			" $('#" << id << "').on('contextmenu', function(event) { if (event.shiftKey) return true; event.preventDefault(); onContextTrack(" << trackID << "); return false; });"
			"});"
			;
		div1.AddChildTag(HtmlTagJavascript(javascript.str()).AddClass("layout_item_script"));
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=trackedit&track=" + trackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=trackaskdelete&track=" + trackIdString + "');"))
			);
		AddChildTag(div2);
	}
};

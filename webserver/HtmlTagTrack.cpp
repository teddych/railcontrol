#include <sstream>

#include "datamodel/loco.h"
#include "webserver/HtmlTagTrack.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagTrack::HtmlTagTrack(const Manager& manager, const datamodel::Track* track)
	{
		layoutPosition_t posX;
		layoutPosition_t posY;
		layoutPosition_t posZ;
		layoutItemSize_t w;
		layoutItemSize_t h;
		layoutRotation_t r;
		track->Position(posX, posY, posZ, w, h, r);
		trackType_t type = track->GetType();
		unsigned int layoutPosX = posX * EdgeLength;
		unsigned int layoutPosY = posY * EdgeLength;

		const string& trackName = track->GetName();
		bool occupied = track->GetFeedbackStateDelayed() == FeedbackStateOccupied;

		locoID_t locoID = track->GetLocoDelayed();
		datamodel::Loco* loco = manager.GetLoco(locoID);
		bool reserved = locoID != LocoNone;

		bool blocked = track->GetBlocked();

		HtmlTag div1("div");
		string trackIdString = to_string(track->GetID());
		string id("t_" + trackIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("track_item");
		string trackClass;
		if (reserved && occupied)
		{
			trackClass = "track_reserved_occupied";
		}
		else if (reserved)
		{
			trackClass = "track_reserved";
		}
		else if (occupied)
		{
			trackClass = "track_occupied";
		}
		else if (blocked)
		{
			trackClass = "track_blocked";
		}
		else
		{
			trackClass = "track_free";
		}

		div1.AddClass(trackClass);
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		std::string image;
		layoutItemSize_t trackHeight = track->GetHeight();
		string layoutHeight = to_string(EdgeLength * trackHeight);

		if (type == TrackTypeTurn)
		{
			image = "<polygon class=\"track\" points=\"0,22 0,13 22,35 13,35\"/>";
		}
		else
		{
			const string& directionSign = track->GetLocoDirection() == DirectionRight ? "&rarr; " : "&larr; ";
			const string& locoName = reserved ? directionSign + loco->GetName() : "";
			image = "<polygon class=\"track\" points=\"13,0 22,0 22," + layoutHeight + " 13," + layoutHeight + "\"/>";
			image += "<text class=\"loconame\" x=\"-" + layoutHeight + "\" y=\"11\" id=\"" + id + "_text_loconame\" transform=\"rotate(270 0,0)\" font-size=\"14\">" + locoName + "</text>";
			image += "<text class=\"trackname\" x=\"-" + layoutHeight + "\" y=\"11\" id=\"" + id + "_text_trackname\" transform=\"rotate(270 0,0)\" font-size=\"14\">" + trackName + "</text>";
		}

		int translateX = 0;
		int translateY = 0;
		if (trackHeight > Height1)
		{
			layoutRotation_t trackRotation = track->GetRotation();
			if (trackRotation == Rotation90 || trackRotation == Rotation270)
			{
				translateX = ((((trackHeight - 1) * EdgeLength) + 1) / 2);
				translateY = (((trackHeight - 1) * EdgeLength) / 2);
			}
			if (trackRotation == Rotation90)
			{
				translateX = -translateX;
				translateY = -translateY;
			}
		}

		div1.AddChildTag(HtmlTag().AddContent("<svg width=\"" + EdgeLengthString + "\" height=\"" + layoutHeight + "\" id=\"" + id + "_img\" style=\"transform:rotate(" + datamodel::LayoutItem::Rotation(track->GetRotation()) + "deg) translate(" + to_string(translateX) + "px," + to_string(translateY) + "px);\">" + image + "</svg>"));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(trackName));
		div1.AddAttribute("oncontextmenu", "return onContextLayoutItem(event, '" + id + "');");
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddClass(reserved ? "loco_known" : "loco_unknown");
		div2.AddClass(blocked ? "track_blocked" : "track_unblocked");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent(trackName))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddClass("track_block").AddContent("Block track").AddAttribute("onClick", "fireRequestAndForget('/?cmd=trackblock&track=" + trackIdString + "&blocked=true');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddClass("track_unblock").AddContent("Unblock track").AddAttribute("onClick", "fireRequestAndForget('/?cmd=trackblock&track=" + trackIdString + "&blocked=false');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddClass("track_release").AddContent("Release track").AddAttribute("onClick", "fireRequestAndForget('/?cmd=trackrelease&track=" + trackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddClass("track_set").AddContent("Set loco").AddAttribute("onClick", "loadPopup('/?cmd=tracksetloco&track=" + trackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddClass("track_start_loco").AddContent("Start loco").AddAttribute("onClick", "fireRequestAndForget('/?cmd=trackstartloco&track=" + trackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddClass("track_stop_loco").AddContent("Stop loco").AddAttribute("onClick", "fireRequestAndForget('/?cmd=trackstoploco&track=" + trackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=trackedit&track=" + trackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=trackaskdelete&track=" + trackIdString + "');"))
			);
		AddChildTag(div2);
	}
};

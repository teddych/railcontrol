#include <sstream>

#include "DataModel/Feedback.h"
#include "WebServer/HtmlTagFeedback.h"

using std::string;
using std::to_string;

namespace WebServer
{
	HtmlTagFeedback::HtmlTagFeedback(const DataModel::Feedback* feedback, layoutPosition_t posX, layoutPosition_t posY)
	{
		DataModel::Feedback::feedbackState_t state = feedback->GetState();

		unsigned int layoutPosX = posX * EdgeLength;
		unsigned int layoutPosY = posY * EdgeLength;
		const string& feedbackName = feedback->GetName();

		HtmlTag div1("div");
		string feedbackIdString = to_string(feedback->GetID());
		string id("f_" + feedbackIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("feedback_item");
		div1.AddClass(state == DataModel::Feedback::FeedbackStateOccupied ? "feedback_occupied" : "feedback_free");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		string image;
		image = "<svg width=\"" + EdgeLengthString + "\" height=\"" + EdgeLengthString + "\" id=\"" + id + "_img\"><circle r=\"12\" cx=\"18\" cy=\"18\" stroke=\"black\" stroke-width=\"2\" class=\"feedback\"/></svg>";
		div1.AddChildTag(HtmlTag().AddContent(image));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(feedbackName + " (pin=" + to_string(feedback->GetPin()) + ")"));
		div1.AddAttribute("onclick", "return onClickFeedback(" + feedbackIdString + ");");
		div1.AddAttribute("oncontextmenu", "return onContextLayoutItem(event, '" + id + "');");
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent(feedbackName))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=feedbackedit&feedback=" + feedbackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=feedbackaskdelete&feedback=" + feedbackIdString + "');"))
			);
		AddChildTag(div2);
	}
};

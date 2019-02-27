#include <sstream>

#include "webserver/HtmlTagFeedback.h"

using std::string;
using std::to_string;

namespace webserver
{
	HtmlTagFeedback::HtmlTagFeedback(const datamodel::Feedback* feedback)
	{
		feedbackState_t state = feedback->GetState();

		unsigned int layoutPosX = feedback->posX * EdgeLength;
		unsigned int layoutPosY = feedback->posY * EdgeLength;

		HtmlTag div1("div");
		string feedbackIdString = to_string(feedback->objectID);
		string id("f_" + feedbackIdString);
		div1.AddAttribute("id", id);
		div1.AddClass("layout_item");
		div1.AddClass("switch_item");
		div1.AddClass(state == FeedbackStateOccupied ? "feedback_occupied" : "feedback_free");
		div1.AddAttribute("style", "left:" + to_string(layoutPosX) + "px;top:" + to_string(layoutPosY) + "px;");
		string image;
		image = "<svg width=\"" + EdgeLengthString + "\" height=\"" + EdgeLengthString + "\" id=\"" + id + "_img\"><circle r=\"12\" cx=\"18\" cy=\"18\" stroke=\"black\" stroke-width=\"2\" class=\"feedback\"/></svg>";
		div1.AddChildTag(HtmlTag().AddContent(image));
		div1.AddChildTag(HtmlTag("span").AddClass("tooltip").AddContent(feedback->Name() + " (pin=" + to_string(feedback->pin) + ")"));
		div1.AddAttribute("onclick", "return onClickFeedback(" + feedbackIdString + ");");
		div1.AddAttribute("oncontextmenu", "return onContextFeedback(event, " + feedbackIdString + ");");
		AddChildTag(div1);

		HtmlTag div2("div");
		div2.AddClass("contextmenu");
		div2.AddAttribute("id", id + "_context");
		div2.AddAttribute("style", "left:" + to_string(layoutPosX + 5) + "px;top:" + to_string(layoutPosY + 30) + "px;");
		div2.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent(feedback->Name()))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Edit").AddAttribute("onClick", "loadPopup('/?cmd=feedbackedit&feedback=" + feedbackIdString + "');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Delete").AddAttribute("onClick", "loadPopup('/?cmd=feedbackaskdelete&feedback=" + feedbackIdString + "');"))
			);
		AddChildTag(div2);
	}
};

#pragma once

#include <string>

#include "datamodel/feedback.h"
#include "datatypes.h"
#include "webserver/HtmlTagLayoutItem.h"

namespace webserver
{
	class HtmlTagFeedback : public HtmlTagLayoutItem
	{
		public:
			HtmlTagFeedback(const datamodel::Feedback* feedback, layoutPosition_t posX, layoutPosition_t posY);
			HtmlTagFeedback(const datamodel::Feedback* feedback)
			:	HtmlTagFeedback(feedback, feedback->GetPosX(), feedback->GetPosY())
			{}
	};
}; // namespace webserver


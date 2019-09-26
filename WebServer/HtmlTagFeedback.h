#pragma once

#include <string>

#include "datatypes.h"
#include "WebServer/HtmlTagLayoutItem.h"

namespace datamodel
{
	class Feedback;
}

namespace WebServer
{
	class HtmlTagFeedback : public HtmlTagLayoutItem
	{
		public:
			HtmlTagFeedback(const datamodel::Feedback* feedback, layoutPosition_t posX, layoutPosition_t posY);
			HtmlTagFeedback(const datamodel::Feedback* feedback)
			:	HtmlTagFeedback(feedback, feedback->GetPosX(), feedback->GetPosY())
			{}
	};
}; // namespace WebServer


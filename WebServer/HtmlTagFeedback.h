#pragma once

#include <string>

#include "DataTypes.h"
#include "WebServer/HtmlTagLayoutItem.h"

namespace DataModel
{
	class Feedback;
}

namespace WebServer
{
	class HtmlTagFeedback : public HtmlTagLayoutItem
	{
		public:
			HtmlTagFeedback(const DataModel::Feedback* feedback, layoutPosition_t posX, layoutPosition_t posY);
			HtmlTagFeedback(const DataModel::Feedback* feedback)
			:	HtmlTagFeedback(feedback, feedback->GetPosX(), feedback->GetPosY())
			{}
	};
}; // namespace WebServer


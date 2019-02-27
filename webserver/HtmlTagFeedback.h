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
			HtmlTagFeedback(const datamodel::Feedback* feedback);
	};
}; // namespace webserver


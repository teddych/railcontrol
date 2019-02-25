#pragma once

#include <string>

#include "datamodel/feedback.h"
#include "datatypes.h"
#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagFeedback : public HtmlTag
	{
		public:
			HtmlTagFeedback(const datamodel::Feedback* feedback);
	};
}; // namespace webserver


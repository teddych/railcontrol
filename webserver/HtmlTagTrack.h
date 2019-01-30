#pragma once

#include <string>

#include "datamodel/track.h"
#include "datatypes.h"
#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagTrack : public HtmlTag
	{
		public:
			HtmlTagTrack(const datamodel::Track* track);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace webserver


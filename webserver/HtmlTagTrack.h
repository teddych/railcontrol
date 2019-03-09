#pragma once

#include <string>

#include "datamodel/track.h"
#include "datatypes.h"
#include "manager.h"
#include "webserver/HtmlTagLayoutItem.h"

namespace webserver
{
	class HtmlTagTrack : public HtmlTagLayoutItem
	{
		public:
			HtmlTagTrack(const Manager& manager, const datamodel::Track* track);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace webserver


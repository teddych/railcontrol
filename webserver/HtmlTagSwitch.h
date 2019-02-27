#pragma once

#include <string>

#include "datamodel/switch.h"
#include "datatypes.h"
#include "webserver/HtmlTagLayoutItem.h"

namespace webserver
{
	class HtmlTagSwitch : public HtmlTagLayoutItem
	{
		public:
			HtmlTagSwitch(const datamodel::Switch* mySwitch);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace webserver


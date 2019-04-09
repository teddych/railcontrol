#pragma once

#include <string>

#include "webserver/HtmlTagLayoutItem.h"

namespace datamodel
{
	class Switch;
}

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


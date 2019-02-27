#pragma once

#include <string>

#include "datamodel/accessory.h"
#include "datatypes.h"
#include "webserver/HtmlTagLayoutItem.h"

namespace webserver
{
	class HtmlTagAccessory : public HtmlTagLayoutItem
	{
		public:
			HtmlTagAccessory(const datamodel::Accessory* accessory);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace webserver


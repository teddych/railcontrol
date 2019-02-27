#pragma once

#include <string>

#include "datamodel/street.h"
#include "datatypes.h"
#include "webserver/HtmlTagLayoutItem.h"

namespace webserver
{
	class HtmlTagStreet : public HtmlTagLayoutItem
	{
		public:
			HtmlTagStreet(const datamodel::Street* street);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace webserver


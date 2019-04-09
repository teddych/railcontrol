#pragma once

#include <string>

#include "webserver/HtmlTagLayoutItem.h"

namespace datamodel
{
	class Street;
}

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


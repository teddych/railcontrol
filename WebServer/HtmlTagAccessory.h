#pragma once

#include <string>

#include "WebServer/HtmlTagLayoutItem.h"

namespace datamodel
{
	class Accessory;
}

namespace WebServer
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


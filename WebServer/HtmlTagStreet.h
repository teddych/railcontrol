#pragma once

#include <string>

#include "WebServer/HtmlTagLayoutItem.h"

namespace datamodel
{
	class Street;
}

namespace WebServer
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
}; // namespace WebServer


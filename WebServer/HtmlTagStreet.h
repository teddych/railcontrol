#pragma once

#include <string>

#include "WebServer/HtmlTagLayoutItem.h"

namespace DataModel
{
	class Street;
}

namespace WebServer
{
	class HtmlTagStreet : public HtmlTagLayoutItem
	{
		public:
			HtmlTagStreet(const DataModel::Street* street);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace WebServer


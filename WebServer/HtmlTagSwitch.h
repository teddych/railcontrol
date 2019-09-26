#pragma once

#include <string>

#include "WebServer/HtmlTagLayoutItem.h"

namespace DataModel
{
	class Switch;
}

namespace WebServer
{
	class HtmlTagSwitch : public HtmlTagLayoutItem
	{
		public:
			HtmlTagSwitch(const DataModel::Switch* mySwitch);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace WebServer


#pragma once

#include <string>

#include "WebServer/HtmlTagLayoutItem.h"

namespace DataModel
{
	class Signal;
}

namespace WebServer
{
	class HtmlTagSignal : public HtmlTagLayoutItem
	{
		public:
			HtmlTagSignal(const DataModel::Signal* signal);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace WebServer


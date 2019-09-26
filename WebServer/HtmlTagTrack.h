#pragma once

#include <string>

#include "manager.h"
#include "WebServer/HtmlTagLayoutItem.h"

namespace DataModel
{
	class Track;
}

namespace WebServer
{
	class HtmlTagTrack : public HtmlTagLayoutItem
	{
		public:
			HtmlTagTrack(const Manager& manager, const DataModel::Track* track);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace WebServer


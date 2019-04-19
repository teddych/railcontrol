#pragma once

#include <string>

#include "webserver/HtmlTagLayoutItem.h"

namespace datamodel
{
	class Signal;
}

namespace webserver
{
	class HtmlTagSignal : public HtmlTagLayoutItem
	{
		public:
			HtmlTagSignal(const datamodel::Signal* signal);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}
	};
}; // namespace webserver


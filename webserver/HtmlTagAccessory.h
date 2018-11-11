#pragma once

#include <string>

#include "datatypes.h"
#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagAccessory : public HtmlTag
	{
		public:
			HtmlTagAccessory(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const accessoryState_t state, const address_t address);

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}

	};
}; // namespace webserver


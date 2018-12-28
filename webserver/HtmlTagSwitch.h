#pragma once

#include <string>

#include "datamodel/switch.h"
#include "datatypes.h"
#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagSwitch : public HtmlTag
	{
		public:
			HtmlTagSwitch(const datamodel::Switch* mySwitch);
			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}

		private:
			void Init(const switchID_t switchID,
				const std::string& name,
				const layoutPosition_t posX,
				const layoutPosition_t posY,
				const layoutPosition_t posZ,
				const layoutRotation_t rotation,
				const switchState_t state,
				const switchType_t type,
				const address_t address
			);
	};
}; // namespace webserver


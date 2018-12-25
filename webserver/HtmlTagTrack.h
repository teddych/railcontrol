#pragma once

#include <string>

#include "datamodel/track.h"
#include "datatypes.h"
#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagTrack : public HtmlTag
	{
		public:
			HtmlTagTrack(const datamodel::Track* track);
			HtmlTagTrack(const trackID_t trackID,
				const std::string& name,
				const layoutPosition_t posX,
				const layoutPosition_t posY,
				const layoutPosition_t posZ,
				const layoutItemSize_t height,
				const std::string rotation,
				const trackType_t type)
			{
				Init(trackID, name, posX, posY, posZ, height, rotation, type);
			}

			virtual HtmlTag AddAttribute(const std::string& name, const std::string& value) override
			{
				childTags[0].AddAttribute(name, value);
				return *this;
			}

		private:
			void Init(const trackID_t trackID,
				const std::string& name,
				const layoutPosition_t posX,
				const layoutPosition_t posY,
				const layoutPosition_t posZ,
				const layoutItemSize_t height,
				const std::string rotation,
				const trackType_t type
			);
	};
}; // namespace webserver


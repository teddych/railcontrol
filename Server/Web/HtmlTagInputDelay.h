#pragma once

#include <string>

#include "Server/Web/HtmlTag.h"

namespace Server { namespace Web
{
	class HtmlTagInputDelay : public HtmlTag
	{
		public:
			HtmlTagInputDelay() = delete;
			HtmlTagInputDelay(HtmlTagInputDelay&) = delete;
			HtmlTagInputDelay& operator=(HtmlTagInputDelay&) = delete;

			HtmlTagInputDelay(const std::string& name, const int value, const int min, const int max);

			inline virtual HtmlTag AddClass(const std::string& className) override
			{
				if (childTags.size() > 1)
				{
					childTags[1].AddClass(className);
				}
				else
				{
					HtmlTag::AddClass(className);
				}
				return *this;
			}
	};
}} // namespace Server::Web
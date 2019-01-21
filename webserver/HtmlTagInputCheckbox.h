#pragma once

#include <string>

#include "webserver/HtmlTagInput.h"

namespace webserver
{
	class HtmlTagInputCheckbox : public HtmlTagInput
	{
		public:
			HtmlTagInputCheckbox(const std::string& name, const std::string& value, const bool checked)
			:	HtmlTagInput("checkbox", name, value)
			{
				if (checked)
				{
					AddAttribute("checked");
				}
				AddClass("checkbox");
			};
	};
};


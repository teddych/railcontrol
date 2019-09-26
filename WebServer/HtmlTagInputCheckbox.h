#pragma once

#include <string>

#include "WebServer/HtmlTagInput.h"

namespace WebServer
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


/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>

#include "Server/Web/HtmlTagInput.h"

namespace Server { namespace Web
{
	class HtmlTagInputIp : public HtmlTagInput
	{
		public:
			HtmlTagInputIp() = delete;
			HtmlTagInputIp(const HtmlTagInputIp&) = delete;
			HtmlTagInputIp& operator=(const HtmlTagInputIp&) = delete;

			// The stored value is shown unchanged, also if it is not a valid IP address.
			// It is only checked by javascript if the user changes it.
			HtmlTagInputIp(const std::string& name,
				const std::string& value = "",
				const HtmlTagInput::Style style = HtmlTagInput::StyleNone)
			:	HtmlTagInput("text",
					name,
					value,
					style)
			{
				AddClass("ip");
				AddAttribute("placeholder", "192.168.0.1");
				AddAttribute("oninput", "checkIpValue('" + name + "');");
			}
	};
}} // namespace Server::Web


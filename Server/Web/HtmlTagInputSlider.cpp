/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2025 by Teddy / Dominik Mahrer - www.railcontrol.org

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

#include "Server/Web/HtmlTagInputSlider.h"

namespace Server { namespace Web
{
	HtmlTagInputSlider::HtmlTagInputSlider(const std::string& name, const unsigned int min, const unsigned int max, const unsigned int value)
	: HtmlTagInput("range", name)
	{
		AddAttribute("min", std::to_string(min));
		AddAttribute("max", std::to_string(max));
		AddAttribute("value", std::to_string(value));
	}
}} // namespace Server::Web

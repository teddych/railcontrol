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

#pragma once

#include <ostream>
#include <string>

#include "Server/Web/ResponseHtml.h"

namespace Server { namespace Web
{
	class ResponseHtmlNotImplemented : public ResponseHtml
	{
		public:
			ResponseHtmlNotImplemented() = delete;
			ResponseHtmlNotImplemented(const ResponseHtmlNotImplemented&) = delete;
			ResponseHtmlNotImplemented& operator=(const ResponseHtmlNotImplemented&) = delete;

			ResponseHtmlNotImplemented(const std::string& method);

			virtual ~ResponseHtmlNotImplemented()
			{
			}
	};
}} // namespace Server::Web


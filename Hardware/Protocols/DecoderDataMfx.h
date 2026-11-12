/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <vector>

#include "DataTypes.h"

// mfx decoder specification:   https://www.skrauss.de/modellbahn/Schienenformat.pdf

namespace Hardware { namespace Protocols
{
	class DecoderDataMfx
	{
		public:
			Address sid = 0;
			MfxUid uid = 0;
			CvValue blockType = 0x01; // Initial block description CA type.
			CvNumber blockStart = 0;
			CvValue groupCount = 0;
			CvValue caCount = 0;
			CvValue caType = 0;
			CvNumber readCv = 0;
			CvIndex readIndex = 0;
			uint32_t manufacturer = 0; // Last byte is the DCC manufacturer ID.
			uint32_t product = 0;
			std::string name;
			std::vector<CvValue> blocks;
			std::vector<CvValue>::size_type nextBlock = 0;
	};
}} // namespace

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

#include "DataModel/LocoFunctions.h"
#include "DataTypes.h"

// mfx decoder specification:   https://www.skrauss.de/modellbahn/Schienenformat.pdf

namespace Hardware { namespace Protocols
{
	// Block 04, CA type 10 (chapter 6.4.4): function symbol of an entry of the function mapping table.
	// Entries are ordered from F0 on (chapter 6.3.1).
	// Version 1: groups of CA types 10, 12, 13: F0-F15, then function when stopped (FS),
	//            function when moving (FF), sensor inputs.
	// Version 2: one CA type 10 per function: F0-F31.
	class DecoderDataMfxFunctionMapping
	{
		public:
			DataModel::LocoFunctionType type = DataModel::LocoFunctionTypeNone; // from bit M
			DataModel::LocoFunctionIcon icon = DataModel::LocoFunctionIconNone;  // from S1 S2
			CvValue group = 0;   // G: function group
			CvValue symbol1 = 0; // S1: function symbol part 1
			CvValue symbol2 = 0; // S2: function symbol part 2
	};

	class DecoderDataMfx
	{
		public:
			Address sid = 0;
			MfxUid uid = 0;
			CvValue blockType = 0x01; // Initial block description CA type.
			CvNumber blockStart = 0;
			CvValue blockVersion = 0; // VV: version of the block structure (chapter 6.3.1)
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
			// Version of block 04 structure, defines the number of functions.
			CvValue functionMappingVersion = 0;
			std::vector<DecoderDataMfxFunctionMapping> functionMappings;

			// Chapter 6.3.1: the function definitions in block 04 are ordered from F0 on.
			// The central units ignore the index table of block 02, so do we.
			inline std::vector<DataModel::LocoFunctionEntry> GetLocoFunctions() const
			{
				// Version 1 contains F0-F15 followed by FS, FF and sensor inputs.
				const size_t maxFunctions = ((functionMappingVersion <= 1) ? 16 : DataModel::NumberOfLocoFunctions);
				std::vector<DataModel::LocoFunctionEntry> out;
				for (size_t nr = 0; (nr < functionMappings.size()) && (nr < maxFunctions); ++nr)
				{
					const DecoderDataMfxFunctionMapping& mapping = functionMappings[nr];
					if (mapping.icon == DataModel::LocoFunctionIconNone)
					{
						continue;
					}
					DataModel::LocoFunctionEntry entry;
					entry.nr = static_cast<DataModel::LocoFunctionNr>(nr);
					entry.type = mapping.type;
					entry.icon = mapping.icon;
					entry.timer = 0;
					out.push_back(entry);
				}
				return out;
			}
	};
}} // namespace

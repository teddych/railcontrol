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

#include <map>
#include <string>

#include "DataTypes.h"
#include "Logger/Logger.h"

class Manager;

namespace Hardware { namespace Protocols
{
	class DecoderMfx
	{
		protected:
			DecoderMfx(Manager* manager, const ControlID controlID, Logger::Logger* logger);

			virtual ~DecoderMfx() = default;

			void ReadDecoderMfx(const Address mfxSid);

			bool ParseResponseMfxLocoName(const Address address,
				const Protocol protocol,
				const CvNumber cv,
				const CvValue value);

			Manager* manager;
			Logger::Logger* logger;
			ControlID controlID;

		private:
			static const uint8_t MfxLocoNameCAType = 0x18;

			struct MfxDecoder
			{
				std::string name;
				MfxUid uid = 0;
				uint8_t nameReadIndex = 0;
			};

			static std::string MfxFallbackName(const Address address);

			void SaveMfxLoco(const MfxUid mfxUid, const Address sid, const std::string& name);

			std::map<Address,MfxDecoder> mfxDecoders;
	};
}} // namespace

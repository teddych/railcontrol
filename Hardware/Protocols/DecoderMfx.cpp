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

#include "DataModel/LocoConfig.h"
#include "Hardware/Protocols/DecoderMfx.h"
#include "Manager.h"

using std::string;

namespace Hardware { namespace Protocols
{
	DecoderMfx::DecoderMfx(Manager* manager, const ControlID controlID, Logger::Logger* logger)
	: manager(manager),
		logger(logger),
		controlID(controlID)
	{
	}

	void DecoderMfx::ReadDecoderMfx(const Address mfxSid)
	{
		mfxDecoders[mfxSid] = MfxDecoder();
		// temporary workaround: read loco name from mfx decoder (CV 3) to get the mfx uid
		manager->ProgramRead(controlID, ProgramModeMfx, mfxSid, 0x0003);
	}

	bool DecoderMfx::ParseResponseMfxLocoName(const Address mfxSid,
		const Protocol protocol,
		const CvNumber cv,
		const CvValue value)
	{
		if ((protocol != ProtocolMFX) || ((cv & 0x3FF) != 3))
		{
			return false;
		}

		const std::map<Address,MfxDecoder>::iterator found = mfxDecoders.find(mfxSid);
		if (found == mfxDecoders.end())
		{
			return false;
		}

		MfxDecoder& decoder = found->second;
		const uint8_t index = cv >> 10;
		if ((index != decoder.nameReadIndex) || (index > 16))
		{
			return false;
		}

		if (index == 0)
		{
			if (value != MfxLocoNameCAType)
			{
				mfxDecoders.erase(found);
				return false;
			}
		}
		else
		{
			if ((value >= 0x20) && (value != 0x7F))
			{
				decoder.name += static_cast<char>(value);
			}

			if ((value == 0) || (index == 16))
			{
				logger->Debug("Loco name of mfx loco {0} is \"{1}\"", mfxSid, decoder.name);
				decoder.nameReadIndex = 17;
				return true;
			}
		}

		++decoder.nameReadIndex;
		manager->ProgramRead(controlID, ProgramModeMfx, mfxSid, (static_cast<CvNumber>(decoder.nameReadIndex) << 10) | 3);
		return true;
	}

	string DecoderMfx::MfxFallbackName(const Address address)
	{
		return "mfx loco " + std::to_string(address);
	}

	void DecoderMfx::SaveMfxLoco(const MfxUid mfxUid, const Address sid, const string& name)
	{
		DataModel::LocoConfig loco = manager->GetLocoByMfxUid(mfxUid);
		string result;
		manager->LocoSave(loco.GetLocoID(),
			name.empty() ? MfxFallbackName(sid) : name,
			controlID,
			loco.GetMatchKey(),
			ProtocolMFX,
			sid,
			mfxUid,
			loco.GetLocoID(),
			loco.GetLength(),
			loco.GetPushpull(),
			loco.GetMaxSpeed(),
			loco.GetTravelSpeed(),
			loco.GetReducedSpeed(),
			loco.GetCreepingSpeed(),
			loco.GetPropulsion(),
			loco.GetTrainType(),
			loco.GetImage(),
			loco.GetCs2Image(),
			loco.GetFunctionStates(),
			result);
	}
}} // namespace

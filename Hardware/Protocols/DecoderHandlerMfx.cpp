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
#include "Hardware/Protocols/DecoderHandlerMfx.h"
#include "Manager.h"
#include <algorithm>

using std::string;

namespace Hardware { namespace Protocols
{
	DecoderHandlerMfx::DecoderHandlerMfx(Manager* manager, const ControlID controlID, Logger::Logger* logger)
	: manager(manager),
		logger(logger),
		controlID(controlID)
	{
	}

	void DecoderHandlerMfx::ReadMfxDecoder(const Address mfxSid)
	{
		mfxDecoders[mfxSid] = DecoderDataMfx();
		DecoderDataMfx& decoder = mfxDecoders[mfxSid];
		decoder.sid = mfxSid;
		ReadCVValue(decoder, 0, 0);
	}

	bool DecoderHandlerMfx::ParseMfxCVValue(const Address mfxSid,
		CvNumber cv,
		const CvValue value)
	{
		const std::map<Address,DecoderDataMfx>::iterator found = mfxDecoders.find(mfxSid);
		if (found == mfxDecoders.end())
		{
			return false;
		}

		DecoderDataMfx& decoder = found->second;
		const CvIndex index = cv >> 10;
		cv &= 0x3FF;

		if ((index != decoder.readIndex) || (cv != decoder.readCv))
		{
			return false;
		}

		// Chapter 6.3.1: index 0 identifies the CA, not the block.
		if (index == 0)
		{
			decoder.caType = value;
		}

		if (decoder.caType == MfxDecoderCATypeBlockDescription)
		{
			return ParseBlockDescription(decoder, cv, index, value);
		}

		if (cv == decoder.blockStart)
		{
			return false; // A block must start with its description CA.
		}

		switch (decoder.blockType)
		{
			case MfxDecoderBlockDecoderProperties:
				switch (decoder.caType)
				{
					case MfxDecoderDecoderPropertiesCATypeManufacturer:
						return ParseDecoderPropertiesManufacturer(decoder, cv, index, value);

					case MfxDecoderDecoderPropertiesCATypeBlockTable:
						return ParseDecoderPropertiesBlockTable(decoder, cv, index, value);

					case MfxDecoderDecoderPropertiesCATypeLocoName:
						return ParseDecoderPropertiesLocoName(decoder, cv, index, value);

					default:
						// Skip unsupported CAs, but continue reading the block.
						ReadNextCA(decoder, cv);
						return true;
				}

			default:
				// Skip unsupported block, but continue reading the next block.
				ReadNextBlock(decoder);
				return true;
		}
	}

	void DecoderHandlerMfx::ReadNextCA(DecoderDataMfx& decoder,
		const CvNumber cv)
	{
		// The description is followed by GG groups of LL CAs (chapter 6.3.1).
		const CvNumber blockEnd = decoder.blockStart + 1 + (decoder.groupCount * decoder.caCount);
		const CvNumber nextCv = cv + 1;
		if ((nextCv < blockEnd) && (nextCv <= 0x3FF))
		{
			ReadCVValue(decoder, nextCv, 0);
			return;
		}

		ReadNextBlock(decoder);
	}

	void DecoderHandlerMfx::ReadNextBlock(DecoderDataMfx& decoder)
	{
		if ((decoder.nextBlock < decoder.blocks.size()))
		{
			// Table entries omit the two zero low bits of the block address.
			decoder.blockStart = static_cast<CvNumber>(decoder.blocks[decoder.nextBlock++]) * 4;
			decoder.blockType = 0;
			decoder.groupCount = 0;
			decoder.caCount = 0;
			ReadCVValue(decoder, decoder.blockStart, 0);
			return;
		}

		// No more blocks: reject responses until a new read is started.
		decoder.readCv = 0x400;
		decoder.readIndex = 0;

		// propagate data
		manager->DecoderDataMfx(decoder);
	}

	bool DecoderHandlerMfx::ParseBlockDescription(DecoderDataMfx& decoder,
		const CvNumber cv,
		const CvIndex index,
		const CvValue value)
	{
		switch (index)
		{
			case 0:
				if ((value != MfxDecoderCATypeBlockDescription)
					|| (cv != decoder.blockStart)
					|| ((cv & 0x03) != 0))
				{
					return false;
				}
				ReadCVValue(decoder, cv, 1);
				return true;

			case 1:
				decoder.blockType = value;
				ReadCVValue(decoder, cv, 4);
				return true;

			case 4:
				decoder.groupCount = value;
				ReadCVValue(decoder, cv, 5);
				return true;

			case 5:
				decoder.caCount = value;
				if ((static_cast<uint32_t>(decoder.blockStart) + 1
					+ (static_cast<uint32_t>(decoder.groupCount) * decoder.caCount)) > 0x400)
				{
					return false;
				}

				ReadNextCA(decoder, cv);
				return true;

			default:
				return false;
		}
	}

	bool DecoderHandlerMfx::ParseDecoderPropertiesManufacturer(DecoderDataMfx& decoder,
		const CvNumber cv,
		const CvIndex index,
		const CvValue value)
	{
		// Chapter 6.4.1: index 1-4 manufacturer ID, index 5-8 product ID, MSB first.
		if (index == 0)
		{
			if (value != MfxDecoderDecoderPropertiesCATypeManufacturer)
			{
				return false;
			}
			decoder.manufacturer = 0;
			decoder.product = 0;

			// We ignore index 1-3 because they are always 0 for known manufacturers.
			ReadCVValue(decoder, cv, 4);
			return true;
		}
		else if (index <= 4)
		{
			decoder.manufacturer = (decoder.manufacturer << 8) | value;
		}
		else if (index <= 8)
		{
			decoder.product = (decoder.product << 8) | value;
		}
		else
		{
			return false;
		}

		if (index == 8)
		{
			ReadNextCA(decoder, cv);
			return true;
		}

		ReadCVValue(decoder, cv, index + 1);
		return true;
	}

	bool DecoderHandlerMfx::ParseDecoderPropertiesBlockTable(DecoderDataMfx& decoder,
		const CvNumber cv,
		const CvIndex index,
		const CvValue value)
	{
		if (index == 0)
		{
			if (value != MfxDecoderDecoderPropertiesCATypeBlockTable)
			{
				return false;
			}

			ReadCVValue(decoder, cv, 1);
			return true;
		}

		if (value == 0)
		{
			ReadNextCA(decoder, cv);
			return true;
		}

		// Ignore duplicate references to avoid revisiting blocks.
		if (std::find(decoder.blocks.begin(), decoder.blocks.end(), value) == decoder.blocks.end())
		{
			decoder.blocks.push_back(value);
		}

		if (index >= 63)
		{
			return false; // Do not overflow the 6-bit index on an unterminated table.
		}

		ReadCVValue(decoder, cv, index + 1);
		return true;
	}

	bool DecoderHandlerMfx::ParseDecoderPropertiesLocoName(DecoderDataMfx& decoder,
		const CvNumber cv,
		const CvIndex index,
		const CvValue value)
	{
		if (index > 16)
		{
			return false;
		}

		if (index == 0)
		{
			if (value != MfxDecoderDecoderPropertiesCATypeLocoName)
			{
				mfxDecoders.erase(decoder.sid);
				return false;
			}
			decoder.name.clear();
		}
		else
		{
			if ((value >= 0x20) && (value != 0x7F))
			{
				decoder.name += static_cast<char>(value);
			}

			if ((value == 0) || (index == 16))
			{
				ReadNextCA(decoder, cv);
				return true;
			}
		}

		ReadCVValue(decoder, cv, index + 1);
		return true;
	}

	string DecoderHandlerMfx::MfxFallbackName(const Address address)
	{
		return "mfx loco " + std::to_string(address);
	}

	void DecoderHandlerMfx::ReadCVValue(DecoderDataMfx& decoder, const CvNumber cv, const CvIndex index)
	{
		decoder.readCv = cv;
		decoder.readIndex = index;
		manager->ProgramRead(controlID, ProgramModeMfx, decoder.sid, (static_cast<CvNumber>(index) << 10) | cv);
	}

	void DecoderHandlerMfx::SaveMfxLoco(const DecoderDataMfx& decoder)
	{
		DataModel::LocoConfig loco = manager->GetLocoByMfxUid(decoder.uid);
		loco.SetName(decoder.name.empty() ? MfxFallbackName(decoder.sid) : decoder.name);
		loco.SetControlID(controlID);
		loco.SetProtocol(ProtocolMFX);
		loco.SetAddress(decoder.sid);
		loco.SetMfxUid(decoder.uid);
		loco.SetServerAddress(loco.GetLocoID());
		string result;
		manager->LocoSave(loco, result);
	}
}} // namespace

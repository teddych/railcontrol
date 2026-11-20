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

	void DecoderHandlerMfx::ReadMfxDecoder(const Address mfxSid, const MfxUid mfxUid)
	{
		mfxDecoders[mfxSid] = DecoderDataMfx();
		DecoderDataMfx& decoder = mfxDecoders[mfxSid];
		decoder.sid = mfxSid;
		decoder.uid = mfxUid;
		ReadCVValue(decoder, 0, 0, 4);
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
			// While reading a decoder, ignore bytes of multibyte reads that are not needed anymore.
			return (decoder.readCv <= 0x3FF);
		}

		// Chapter 6.3.1: index 0 identifies the CA, not the block.
		if (index == 0)
		{
			decoder.caType = value;
		}

		if (decoder.caType == MfxDecoderCATypeBlockDescription)
		{
			return ParseBlockDescription(decoder, value);
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
						return ParseDecoderPropertiesManufacturer(decoder, value);

					case MfxDecoderDecoderPropertiesCATypeBlockTable:
						return ParseDecoderPropertiesBlockTable(decoder, value);

					case MfxDecoderDecoderPropertiesCATypeLocoName:
						return ParseDecoderPropertiesLocoName(decoder, value);

					default:
						// Skip unsupported CAs, but continue reading the block.
						ReadNextCA(decoder);
						return true;
				}

			case MfxDecoderBlockFunctionMapping:
				switch (decoder.caType)
				{
					case MfxDecoderFunctionMappingCATypeSymbol:
						return ParseFunctionMappingSymbol(decoder, value);

					default:
						// Skip unsupported CAs, but continue reading the block.
						ReadNextCA(decoder);
						return true;
				}

			default:
				// Skip unsupported block, but continue reading the next block.
				ReadNextBlock(decoder);
				return true;
		}
	}

	void DecoderHandlerMfx::ReadNextCA(DecoderDataMfx& decoder)
	{
		// Multibyte reads always read 4 indexes: wait for the last requested index before requesting the next CA.
		if ((decoder.readIndex & 0x03) != 0x03)
		{
			ReadNextIndex(decoder);
			return;
		}

		// The description is followed by GG groups of LL CAs (chapter 6.3.1).
		const CvNumber blockEnd = decoder.blockStart + 1 + (decoder.groupCount * decoder.caCount);
		const CvNumber nextCv = decoder.readCv + 1;
		if ((nextCv < blockEnd) && (nextCv <= 0x3FF))
		{
			ReadCVValue(decoder, nextCv, 0, 4);
			return;
		}

		ReadNextBlock(decoder);
	}

	void DecoderHandlerMfx::ReadNextBlock(DecoderDataMfx& decoder)
	{
		// Multibyte reads always read 4 indexes: wait for the last requested index before requesting the next block.
		if ((decoder.readIndex & 0x03) != 0x03)
		{
			ReadNextIndex(decoder);
			return;
		}

		if ((decoder.nextBlock < decoder.blocks.size()))
		{
			// Table entries omit the two zero low bits of the block address.
			decoder.blockStart = static_cast<CvNumber>(decoder.blocks[decoder.nextBlock++]) * 4;
			decoder.blockType = 0;
			decoder.blockVersion = 0;
			decoder.groupCount = 0;
			decoder.caCount = 0;
			ReadCVValue(decoder, decoder.blockStart, 0, 4);
			return;
		}

		// No more blocks: reject responses until a new read is started.
		decoder.readCv = 0x400;
		decoder.readIndex = 0;

		// propagate data
		manager->DecoderDataMfx(decoder);

		SaveMfxLoco(decoder);
	}

	bool DecoderHandlerMfx::ParseBlockDescription(DecoderDataMfx& decoder,
		const CvValue value)
	{
		const CvIndex index = decoder.readIndex;

		switch (index)
		{
			case 0:
				if ((decoder.readCv != decoder.blockStart)
					|| ((decoder.readCv & 0x03) != 0))
				{
					return false;
				}
				ReadNextIndex(decoder);
				return true;

			case 1:
				decoder.blockType = value;
				logger->Info(Languages::TextReadingMfxBlockType, decoder.blockType);
				ReadNextIndex(decoder);
				return true;

			case 2:
				decoder.blockVersion = value;
				if (decoder.blockType == MfxDecoderBlockFunctionMapping)
				{
					decoder.functionMappingVersion = value;
				}
				ReadNextIndex(decoder);
				return true;

			case 3:
				// data not needed, but requested by the multibyte read
				ReadNextIndex(decoder);
				return true;

			case 4:
				decoder.groupCount = value;
				ReadNextIndex(decoder);
				return true;

			case 5:
				decoder.caCount = value;
				if ((static_cast<uint32_t>(decoder.blockStart) + 1
					+ (static_cast<uint32_t>(decoder.groupCount) * decoder.caCount)) > 0x400)
				{
					return false;
				}

				ReadNextCA(decoder);
				return true;

			case 6:
			case 7:
				// data not needed, but requested by the multibyte read
				ReadNextCA(decoder);
				return true;

			default:
				return false;
		}
	}

	bool DecoderHandlerMfx::ParseDecoderPropertiesManufacturer(DecoderDataMfx& decoder,
		const CvValue value)
	{
		const CvIndex index = decoder.readIndex;

		// Chapter 6.4.1: index 1-4 manufacturer ID, index 5-8 product ID, MSB first.
		if (index == 0)
		{
			decoder.manufacturer = 0;
			decoder.product = 0;
		}
		else if (index <= 4)
		{
			decoder.manufacturer = (decoder.manufacturer << 8) | value;
		}
		else if (index <= 8)
		{
			decoder.product = (decoder.product << 8) | value;
			if (index == 8)
			{
				ReadNextCA(decoder);
				return true;
			}
		}
		else if (index <= 11)
		{
			// Index 9-11 are requested by the multibyte read but not needed.
			ReadNextCA(decoder);
			return true;
		}
		else
		{
			return false;
		}

		ReadNextIndex(decoder);
		return true;
	}

	bool DecoderHandlerMfx::ParseDecoderPropertiesBlockTable(DecoderDataMfx& decoder,
		const CvValue value)
	{
		const CvIndex index = decoder.readIndex;

		if (index == 0)
		{
			ReadNextIndex(decoder);
			return true;
		}

		// The table is terminated by 0.
		if (value == 0)
		{
			ReadNextCA(decoder);
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

		ReadNextIndex(decoder);
		return true;
	}

	bool DecoderHandlerMfx::ParseDecoderPropertiesLocoName(DecoderDataMfx& decoder,
		const CvValue value)
	{
		const CvIndex index = decoder.readIndex;

		// Index 1-16 contain the name, index 17-19 are requested by the multibyte read but not needed.
		if (index > 19)
		{
			return false;
		}

		if (index == 0)
		{
			decoder.name.clear();
		}
		else
		{
			if ((index <= 16) && (value >= 0x20) && (value != 0x7F))
			{
				decoder.name += static_cast<char>(value);
			}

			// The name is terminated by 0 or has a maximum of 16 characters.
			if ((value == 0) || (index >= 16))
			{
				ReadNextCA(decoder);
				return true;
			}
		}

		ReadNextIndex(decoder);
		return true;
	}

	bool DecoderHandlerMfx::ParseFunctionMappingSymbol(DecoderDataMfx& decoder,
		const CvValue value)
	{
		const CvIndex index = decoder.readIndex;

		// Chapter 6.4.4, CA type 10: index 1 M GGGGGGG, index 2 S1, index 3 S2.
		switch (index)
		{
			case 0:
				decoder.functionMappings.push_back(DecoderDataMfxFunctionMapping());
				ReadNextIndex(decoder);
				return true;

			case 1:
			{
				DecoderDataMfxFunctionMapping& mapping = decoder.functionMappings.back();
				mapping.type = ((value & 0x80) ? DataModel::LocoFunctionTypeMoment : DataModel::LocoFunctionTypePermanent);
				mapping.group = value & 0x7F;
				ReadNextIndex(decoder);
				return true;
			}

			case 2:
				decoder.functionMappings.back().symbol1 = value;
				ReadNextIndex(decoder);
				return true;

			case 3:
			{
				DecoderDataMfxFunctionMapping& mapping = decoder.functionMappings.back();
				mapping.symbol2 = value;
				mapping.icon = MfxSymbolToLocoFunctionIcon(mapping.symbol1, mapping.symbol2);
				if (mapping.icon == DataModel::LocoFunctionIconNone)
				{
					mapping.type = DataModel::LocoFunctionTypeNone;
				}
				ReadNextCA(decoder);
				return true;
			}

			default:
				return false;
		}
	}

	DataModel::LocoFunctionIcon DecoderHandlerMfx::MfxSymbolToLocoFunctionIcon(const CvValue symbol1,
		const CvValue symbol2)
	{
		// Chapter 6.4.4: the function group G is not relevant, the symbol is defined by S1 and S2.
		// The choice of icons is aligned with LocoFunctionMapCs2ToRailControl in MaerklinCANCommon.cpp.
		switch (symbol1)
		{
			case 0x00:
				return DataModel::LocoFunctionIconNone;

			case 0x03: // head light
				switch (symbol2)
				{
					case 0x40:
						return DataModel::LocoFunctionIconCabLight2;

					case 0x80:
						return DataModel::LocoFunctionIconCabLight1;

					default:
						return DataModel::LocoFunctionIconLight;
				}

			case 0x04: // interior light
				switch (symbol2)
				{
					case 0x04:
						return DataModel::LocoFunctionIconInteriorLight2;

					case 0x05:
						return DataModel::LocoFunctionIconTableLight1;

					case 0x06:
						return DataModel::LocoFunctionIconTableLight2;

					case 0x07:
						return DataModel::LocoFunctionIconTableLight3;

					default:
						return DataModel::LocoFunctionIconInteriorLight1;
				}

			case 0x05: // exterior light
				switch (symbol2)
				{
					case 0x01:
						return DataModel::LocoFunctionIconLocomotiveNumberIndicator;

					case 0x02:
						return DataModel::LocoFunctionIconTrainDestinationIndicator;

					case 0x03:
						return DataModel::LocoFunctionIconEngineLight;

					case 0x06:
						return DataModel::LocoFunctionIconBlinkingLight;

					case 0x07:
						return DataModel::LocoFunctionIconStairsLight;

					case 0x09:
						return DataModel::LocoFunctionIconFireBox;

					default:
						return DataModel::LocoFunctionIconLight;
				}

			case 0x07: // sound
				switch (symbol2)
				{
					case 0x01:
						return DataModel::LocoFunctionIconBreak1;

					case 0x02:
						return DataModel::LocoFunctionIconPanto;

					case 0x05:
						return DataModel::LocoFunctionIconCoupler;

					case 0x06:
						return DataModel::LocoFunctionIconRailJoint;

					case 0x08:
						return DataModel::LocoFunctionIconWhistle2;

					case 0x09:
						return DataModel::LocoFunctionIconGearBox;

					case 0x0A:
						return DataModel::LocoFunctionIconSteamBlowOut;

					case 0x0D:
					case 0x10:
						return DataModel::LocoFunctionIconShakingRust;

					case 0x0E:
						return DataModel::LocoFunctionIconGenerator;

					case 0x0F:
						return DataModel::LocoFunctionIconBufferPush;

					case 0x11:
						return DataModel::LocoFunctionIconMusic1;

					case 0x13:
						return DataModel::LocoFunctionIconSteamBlow;

					case 0x14:
						return DataModel::LocoFunctionIconEngine1;

					case 0x2E:
						return DataModel::LocoFunctionIconSand;

					case 0x2F:
						return DataModel::LocoFunctionIconNoBreak;

					default:
						return DataModel::LocoFunctionIconSoundGeneral;
				}

			case 0x08: // background sound
				return DataModel::LocoFunctionIconRunning1;

			case 0x09: // announcement
				switch (symbol2)
				{
					case 0x02:
						return DataModel::LocoFunctionIconStationAnnouncement2;

					case 0x04:
						return DataModel::LocoFunctionIconStationAnnouncement3;

					case 0x05:
						return DataModel::LocoFunctionIconSpeak;

					default:
						return DataModel::LocoFunctionIconStationAnnouncement1;
				}

			case 0x0A: // shunting mode
				return DataModel::LocoFunctionIconShuntingMode;

			case 0x0B: // direct control (inertia off)
				return DataModel::LocoFunctionIconInertia;

			case 0x20: // telex
				switch (symbol2)
				{
					case 0x40:
						return DataModel::LocoFunctionIconTelex2;

					case 0x80:
						return DataModel::LocoFunctionIconTelex1;

					default:
						return DataModel::LocoFunctionIconTelex12;
				}

			case 0x21:
				return DataModel::LocoFunctionIconSmokeGenerator;

			case 0x22: // pantograph
				switch (symbol2)
				{
					case 0x40:
						return DataModel::LocoFunctionIconPanto2;

					case 0x80:
						return DataModel::LocoFunctionIconPanto1;

					default:
						return DataModel::LocoFunctionIconPanto12;
				}

			case 0x23: // high beam
				switch (symbol2)
				{
					case 0x40:
						return DataModel::LocoFunctionIconHeadlightHighBeamReverse;

					default:
						return DataModel::LocoFunctionIconHeadlightHighBeamForward;
				}

			case 0x24:
				return DataModel::LocoFunctionIconBell;

			case 0x25:
				return DataModel::LocoFunctionIconHorn1;

			case 0x26:
				return DataModel::LocoFunctionIconWhistle1;

			case 0x27: // doors
				return ((symbol2 == 0x01) ? DataModel::LocoFunctionIconCloseDoor : DataModel::LocoFunctionIconOpenDoor);

			case 0x28:
				return DataModel::LocoFunctionIconFan;

			case 0x29: // pumps
				switch (symbol2)
				{
					case 0x02:
						return DataModel::LocoFunctionIconFan2;

					case 0x04:
						return DataModel::LocoFunctionIconFillWater;

					case 0x05:
						return DataModel::LocoFunctionIconWaterPump;

					default:
						return DataModel::LocoFunctionIconAirPump;
				}

			case 0x2A:
				return DataModel::LocoFunctionIconShovelCoal;

			case 0x2B: // crane
				switch (symbol2)
				{
					case 0x01:
						return DataModel::LocoFunctionIconUpDown1;

					case 0x02:
						return DataModel::LocoFunctionIconUpDown2;

					case 0x03:
						return DataModel::LocoFunctionIconTurnRight;

					case 0x04:
						return DataModel::LocoFunctionIconLeftRight;

					case 0x05:
						return DataModel::LocoFunctionIconCrane;

					case 0x06:
						return DataModel::LocoFunctionIconMagnet;

					case 0x07:
						return DataModel::LocoFunctionIconUp;

					case 0x08:
						return DataModel::LocoFunctionIconDown;

					case 0x09:
						return DataModel::LocoFunctionIconLeft;

					case 0x0A:
						return DataModel::LocoFunctionIconRight;

					case 0x0C:
						return DataModel::LocoFunctionIconTurnLeft;

					default:
						return DataModel::LocoFunctionIconCraneHook;
				}

			case 0x2C:
				return ((symbol2 == 0x01) ? DataModel::LocoFunctionIconNoSound : DataModel::LocoFunctionIconUp);

			case 0x02: // undefined
			case 0x06: // mechanical function
			case 0x1F: // user defined
			default:
				return DataModel::LocoFunctionIconDefault;
		}
	}

	string DecoderHandlerMfx::MfxFallbackName(const Address address)
	{
		return "mfx loco " + std::to_string(address);
	}

	void DecoderHandlerMfx::ReadCVValue(DecoderDataMfx& decoder, const CvNumber cv, const CvIndex index, const uint8_t count)
	{
		decoder.readCv = cv;
		decoder.readIndex = index;
		ReadMfxCVValue(decoder.sid, cv, index, count);
	}

	void DecoderHandlerMfx::ReadNextIndex(DecoderDataMfx& decoder)
	{
		++decoder.readIndex;

		// Multibyte reads always start at a multiple of 4 and read 4 indexes.
		// Otherwise the index is already requested by a multibyte read: just wait for the value.
		if ((decoder.readIndex & 0x03) == 0)
		{
			ReadMfxCVValue(decoder.sid, decoder.readCv, decoder.readIndex, 4);
		}
	}

	void DecoderHandlerMfx::SaveMfxLoco(const DecoderDataMfx& decoder)
	{
		// Without UID (e.g. manual read via SID) the decoder can not be identified.
		if (decoder.uid == 0)
		{
			return;
		}

		// Save decoder only as new loco if no loco with this mfx UID exists yet.
		if (manager->GetLocoByMfxUid(decoder.uid).GetType() == LocoTypeLoco)
		{
			return;
		}

		DataModel::LocoConfig loco(LocoTypeLoco);
		loco.SetName(decoder.name.empty() ? MfxFallbackName(decoder.sid) : decoder.name);
		loco.SetControlID(controlID);
		loco.SetProtocol(ProtocolMFX);
		loco.SetAddress(decoder.sid);
		loco.SetMfxUid(decoder.uid);
		loco.SetServerAddress(loco.GetLocoID());
		const std::vector<DataModel::LocoFunctionEntry> functions = decoder.GetLocoFunctions();
		if (!functions.empty())
		{
			loco.SetFunctionStates(functions);
		}
		string result;
		manager->LocoSave(loco, result);
	}
}} // namespace

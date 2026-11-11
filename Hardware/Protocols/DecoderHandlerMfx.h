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
#include "Hardware/Protocols/DecoderDataMfx.h"
#include "Logger/Logger.h"

class Manager;

namespace Hardware { namespace Protocols
{
	class DecoderHandlerMfx
	{
		protected:
			DecoderHandlerMfx(Manager* manager, const ControlID controlID, Logger::Logger* logger);

			virtual ~DecoderHandlerMfx() = default;

			void ReadMfxDecoder(const Address mfxSid);

			bool ParseMfxCVValue(const Address mfxSid,
				CvNumber cv,
				const CvValue value);

			Manager* manager;
			Logger::Logger* logger;
			ControlID controlID;

		private:
			enum MfxDecoderBlock : CvValue
			{
				MfxDecoderBlockDecoderProperties = 0x01,
				MfxDecoderBlockFunctions = 0x02,
				MfxDecoderBlockAutomaticFunctions = 0x03,
				MfxDecoderBlockFunctionMapping = 0x04,
				MfxDecoderBlockDrivingProperties = 0x05,
				MfxDecoderBlockHardwareFunctions = 0x06,
				MfxDecoderBlockAdditionalProtocols = 0x07,
				MfxDecoderBlockSoundSettings = 0x08,
				MfxDecoderBlockSpecialOptions = 0x09,
				MfxDecoderBlockMfxPlus = 0x0A
			};

			// CA type 01 has the same meaning in every block.
			static const CvValue MfxDecoderCATypeBlockDescription = 0x01;

			enum MfxDecoderDecoderPropertiesCAType : CvValue
			{
				MfxDecoderDecoderPropertiesCATypeManufacturer = 0x10,
				MfxDecoderDecoderPropertiesCATypeProduction = 0x11,
				MfxDecoderDecoderPropertiesCATypeBootloader = 0x12,
				MfxDecoderDecoderPropertiesCATypeFirmware = 0x13,
				MfxDecoderDecoderPropertiesCATypeProtocol = 0x14,
				MfxDecoderDecoderPropertiesCATypeUnknown15 = 0x15,
				MfxDecoderDecoderPropertiesCATypeLocoIdentification = 0x16,
				MfxDecoderDecoderPropertiesCATypeBlockTable = 0x17,
				MfxDecoderDecoderPropertiesCATypeLocoName = 0x18,
				MfxDecoderDecoderPropertiesCATypeUserData = 0x19,
				MfxDecoderDecoderPropertiesCATypeUnknown1A = 0x1A,
				MfxDecoderDecoderPropertiesCATypeHardware = 0x1B
			};

			enum MfxDecoderFunctionsCAType : CvValue
			{
				MfxDecoderFunctionsCATypeDrivefunction = 0x10,
				MfxDecoderFunctionsCATypeSwitchingFunction = 0x11
			};

			enum MfxDecoderAutomaticFunctionsCAType : CvValue
			{
				MfxDecoderAutomaticFunctionsCATypeStopped = 0x10,
				MfxDecoderAutomaticFunctionsCATypeMoving = 0x11,
				MfxDecoderAutomaticFunctionsCATypeSensorInput = 0x12
			};

			enum MfxDecoderFunctionMappingCAType : CvValue
			{
				MfxDecoderFunctionMappingCATypeSymbol = 0x10,
				MfxDecoderFunctionMappingCATypeForward = 0x12,
				MfxDecoderFunctionMappingCATypeReverse = 0x13
			};

			enum MfxDecoderDrivingPropertiesCAType : CvValue
			{
				MfxDecoderDrivingPropertiesCATypeMotors = 0x10,
				MfxDecoderDrivingPropertiesCATypeMotorType = 0x11,
				MfxDecoderDrivingPropertiesCATypeMotorFrequency = 0x12,
				MfxDecoderDrivingPropertiesCATypeAccelerationBraking = 0x13,
				MfxDecoderDrivingPropertiesCATypeTrim = 0x14,
				MfxDecoderDrivingPropertiesCATypeLoadControl = 0x15,
				MfxDecoderDrivingPropertiesCATypeBrakingSection = 0x16,
				MfxDecoderDrivingPropertiesCATypeSpeedCurve = 0x17,
				MfxDecoderDrivingPropertiesCATypeSpeedometer = 0x18,
				MfxDecoderDrivingPropertiesCATypeReverse = 0x19
			};

			enum MfxDecoderHardwareFunctionsCAType : CvValue
			{
				MfxDecoderHardwareFunctionsCATypeOutput = 0x10,
				MfxDecoderHardwareFunctionsCATypeInternal = 0x11,
				MfxDecoderHardwareFunctionsCATypeSound = 0x12
			};

			enum MfxDecoderAdditionalProtocolsCAType : CvValue
			{
				MfxDecoderAdditionalProtocolsCATypeProtocol = 0x10,
				MfxDecoderAdditionalProtocolsCATypeConfiguration = 0x11,
				MfxDecoderAdditionalProtocolsCATypeEnabledFunctions = 0x12,
				// Type 13 holds MM or DCC addresses, depending on the protocol.
				MfxDecoderAdditionalProtocolsCATypeAddresses = 0x13,
				MfxDecoderAdditionalProtocolsCATypeAnalogVoltage = 0x14
			};

			enum MfxDecoderSoundSettingsCAType : CvValue
			{
				MfxDecoderSoundSettingsCATypeVolume = 0x10,
				MfxDecoderSoundSettingsCATypeDieselElectric = 0x11,
				MfxDecoderSoundSettingsCATypeSteam = 0x12,
				MfxDecoderSoundSettingsCATypeSpeed = 0x13,
				MfxDecoderSoundSettingsCATypeRandomSound = 0x14,
				MfxDecoderSoundSettingsCATypeBrakingSound = 0x15,
				MfxDecoderSoundSettingsCATypeAutomaticSounds = 0x16
			};

			enum MfxDecoderSpecialOptionsCAType : CvValue
			{
				MfxDecoderSpecialOptionsCATypeStorageA = 0x10,
				MfxDecoderSpecialOptionsCATypeStorageB = 0x11,
				MfxDecoderSpecialOptionsCATypeOutputPorts = 0x12
			};

			enum MfxDecoderMfxPlusCAType : CvValue
			{
				MfxDecoderMfxPlusCATypeCurrentSupplies = 0x10,
				MfxDecoderMfxPlusCATypeMaximumSupplies = 0x11,
				MfxDecoderMfxPlusCATypeConsumption = 0x12,
				MfxDecoderMfxPlusCATypeReserve = 0x13,
				MfxDecoderMfxPlusCATypeEmergencySpeed = 0x14,
				MfxDecoderMfxPlusCATypeDrivingTime = 0x15,
				MfxDecoderMfxPlusCATypeDistance = 0x16,
				MfxDecoderMfxPlusCATypeLoad = 0x17,
				MfxDecoderMfxPlusCATypeGearRatio = 0x18,
				MfxDecoderMfxPlusCATypeInterval = 0x19,
				MfxDecoderMfxPlusCATypeMode = 0x1A,
				MfxDecoderMfxPlusCATypeCab = 0x1B
			};

			void ReadCVValue(DecoderDataMfx& decoder, const CvNumber cv, const CvIndex index);
			void ReadNextCA(DecoderDataMfx& decoder, const CvNumber cv);
			void ReadNextBlock(DecoderDataMfx& decoder);
			bool ParseBlockDescription(DecoderDataMfx& decoder, const CvNumber cv,
				const CvIndex index, const CvValue value);
			bool ParseDecoderPropertiesManufacturer(DecoderDataMfx& decoder, const CvNumber cv,
				const CvIndex index, const CvValue value);
			bool ParseDecoderPropertiesBlockTable(DecoderDataMfx& decoder, const CvNumber cv,
				const CvIndex index, const CvValue value);
			bool ParseDecoderPropertiesLocoName(DecoderDataMfx& decoder, const CvNumber cv,
				const CvIndex index, const CvValue value);

			static std::string MfxFallbackName(const Address address);

			void SaveMfxLoco(const DecoderDataMfx& decoder);

			std::map<Address,DecoderDataMfx> mfxDecoders;
	};
}} // namespace

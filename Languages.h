/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <map>
#include <string>

#include "DataModel/Accessory.h"
#include "DataTypes.h"

class Languages
{
	public:
		enum textSelector_t : unsigned int
		{
			TextAccessoryAddressDccTooHigh,
			TextAccessoryAddressMmTooHigh,
			TextAccessoryDeleted,
			TextAccessoryIsLocked,
			TextAccessoryStateIsGreen,
			TextAccessoryStateIsRed,
			TextAccessoryUpdated,
			TextAddingFeedback,
			TextAddressMustBeHigherThen0,
			TextConnectionReset,
			TextControlDoesNotExist,
			TextDebounceThreadStarted,
			TextDebounceThreadTerminated,
			TextDebouncer,
			TextDifferentCommuterTypes,
			TextDifferentDirections,
			TextFeedbackChange,
			TextFeedbackDeleted,
			TextFeedbackStateIsOff,
			TextFeedbackStateIsOn,
			TextFeedbackUpdated,
			TextGreen,
			TextHeightIs0,
			TextHsi88Configured,
			TextHsi88ErrorConfiguring,
			TextInvalidDataReceived,
			TextLayer1,
			TextLeft,
			TextLoadedAccessory,
			TextLoadedControl,
			TextLoadedFeedback,
			TextLoadedLayer,
			TextLoadedLoco,
			TextLoadedSignal,
			TextLoadedStreet,
			TextLoadedSwitch,
			TextLoadedTrack,
			TextLocoAddressDccTooHigh,
			TextLocoAddressMmTooHigh,
			TextLocoDirectionIsLeft,
			TextLocoDirectionIsRight,
			TextLocoFunctionIsOff,
			TextLocoFunctionIsOn,
			TextLocoIsOnTrack,
			TextLocoIsReleased,
			TextLocoSpeedIs,
			TextManager,
			TextNoS88Modules,
			TextOff,
			TextOn,
			TextPositionAlreadyInUse,
			TextProtocolNotSupported,
			TextRailControlStarted,
			TextReceivedSignalKill,
			TextReceiverThreadStarted,
			TextRed,
			TextRight,
			TextSaving,
			TextSenderSocketCreated,
			TextSerialNumberIs,
			TextSettingAccessory,
			TextSettingAccessoryWithProtocol,
			TextSettingDirection,
			TextSettingDirectionWithProtocol,
			TextSettingFunction,
			TextSettingFunctionWithProtocol,
			TextSettingSpeed,
			TextSettingSpeedDirectionLight,
			TextSettingSpeedWithProtocol,
			TextSignalDeleted,
			TextSignalIsLocked,
			TextSignalStateIsGreen,
			TextSignalStateIsRed,
			TextSignalUpdated,
			TextStoppingRailControl,
			TextStoppingRequestedBySignal,
			TextStoppingRequestedByWebClient,
			TextStreetDeleted,
			TextStreetIsReleased,
			TextStreetUpdated,
			TextSwitchDeleted,
			TextSwitchIsLocked,
			TextSwitchStateIsStraight,
			TextSwitchStateIsTurnout,
			TextSwitchUpdated,
			TextTerminatingReceiverThread,
			TextTerminatingSenderSocket,
			TextTooManyS88Modules,
			TextTrackDeleted,
			TextTrackStatusIsBlocked,
			TextTrackStatusIsBlockedAndOccupied,
			TextTrackStatusIsBlockedAndReserved,
			TextTrackStatusIsFree,
			TextTrackStatusIsOccupied,
			TextTrackStatusIsReserved,
			TextTrackUpdated,
			TextTrainIsToLong,
			TextTrainIsToShort,
			TextTurningBoosterOff,
			TextTurningBoosterOn,
			TextUnableAddLayer1,
			TextUnableCreateStorageHandler,
			TextUnableToAddAccessory,
			TextUnableToAddFeedback,
			TextUnableToAddLayer,
			TextUnableToAddSignal,
			TextUnableToAddStreet,
			TextUnableToAddSwitch,
			TextUnableToAddTrack,
			TextUnableToBindSocketToPort,
			TextUnableToBindUdpSocket,
			TextUnableToCreatUdpSocketForReceivingData,
			TextUnableToCreatUdpSocketForSendingData,
			TextUnableToCreateSocket,
			TextUnableToExecuteStreet,
			TextUnableToLockStreet,
			TextUnableToMoveAccessory,
			TextUnableToMoveFeedback,
			TextUnableToMoveSignal,
			TextUnableToMoveStreet,
			TextUnableToMoveSwitch,
			TextUnableToMoveTrack,
			TextUnableToReceiveData,
			TextUnableToReserveStreet,
			TextUnableToResolveAddress,
			TextUnableToSendDataToControl,
			TextUnknownAccessory,
			TextUnknownControl,
			TextUnknownFeedback,
			TextUnknownLoco,
			TextUnknownSignal,
			TextUnknownStreet,
			TextUnknownSwitch,
			TextUnknownTrack,
			TextUnloadingControl,
			TextWebServerStarted,
			TextWebServerStopped,
			TextWidthIs0,
			TextZ21Black2012,
			TextZ21Black2013,
			TextZ21DoesNotUnderstand,
			TextZ21SmartRail2012,
			TextZ21Start2016,
			TextZ21Type,
			TextZ21Unknown,
			TextZ21White2013,
			MaxTexts
		};

		enum language_t : unsigned char
		{
			FirstLanguage = 0,
			EN = 0,
			DE,
			ES,
			MaxLanguages
		};

		static void SetDefaultLanguage(std::string& languageString)
		{
			defaultLanguage = ParseLanguage(languageString);
		}

		static language_t ParseLanguage(std::string& languageString);

		static const char* GetText(const textSelector_t selector)
		{
			return GetText(defaultLanguage, selector);
		}

		static const char* GetText(const language_t language, const textSelector_t selector)
		{
			static const char* unknownText = "Unknown Text";

			if (language >= MaxLanguages || selector >= MaxTexts)
			{
				return unknownText;
			}

			return languages[selector][language];
		}

		static const char* GetOnOff(const bool on)
		{
			return GetText(on ? TextOn : TextOff);
		}

		static const char* GetLeftRight(const direction_t direction)
		{
			return GetText(direction == DirectionRight ? TextRight : TextLeft);
		}

		static const char* GetGreenRed(const accessoryState_t state)
		{
			return GetText(state == DataModel::Accessory::AccessoryStateOn ? TextGreen : TextRed);
		}

		static const char* languages[MaxTexts][MaxLanguages];
		static language_t defaultLanguage;
};

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

class Languages
{
	public:
		enum language_t : unsigned char
		{
			FirstLanguage = 0,
			EN = 0,
			DE,
			ES,
			MaxLanguages
		};

		enum textSelector_t : unsigned int
		{
			TextAccessoryAddressDccTooHigh,
			TextAccessoryAddressMmTooHigh,
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
			TextFeedbackState,
			TextHeightIs0,
			TextHsi88Configured,
			TextHsi88ErrorConfiguring,
			TextIsLocked,
			TextLayer1,
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
			TextLocoDirection,
			TextLocoFunction,
			TextLocoSpeed,
			TextManager,
			TextNoS88Modules,
			TextPositionAlreadyInUse,
			TextProtocolNotSupported,
			TextReceivedSignalKill,
			TextSaving,
			TextSerialNumberIs,
			TextStoppingRailControl,
			TextStoppingRequestedBySignal,
			TextStoppingRequestedByWebClient,
			TextTooManyS88Modules,
			TextTrainIsToLong,
			TextTrainIsToShort,
			TextTurningBoosterOff,
			TextTurningBoosterOn,
			TextUnableAddLayer1,
			TextUnableCreateStorageHandler,
			TextUnableToBindSocketToPort,
			TextUnableToCreateSocket,
			TextUnableToExecuteStreet,
			TextUnableToLockStreet,
			TextUnableToReserveStreet,
			TextUnableToResolveAddress,
			TextUnknownAccessory,
			TextUnknownControl,
			TextUnknownFeedback,
			TextUnknownLoco,
			TextUnknownSignal,
			TextUnknownStreet,
			TextUnknownSwitch,
			TextUnknownTrack,
			TextUnloadingControl,
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

		static const char* languages[MaxTexts][MaxLanguages];
		static language_t defaultLanguage;
};

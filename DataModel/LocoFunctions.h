/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <vector>

#include "DataModel/Serializable.h"

namespace DataModel
{
	class LocoFunctions : private Serializable
	{
		public:
			typedef uint8_t LocoFunctionNr;

			enum LocoFunctionState : unsigned char
			{
				LocoFunctionStateOff = 0,
				LocoFunctionStateOn
			};

			enum LocoFunctionType : uint8_t
			{
				LocoFunctionTypeNone = 0,
				LocoFunctionTypePermanent = 1,
				LocoFunctionTypeOnce = 2,
				LocoFunctionTypeFlashing = 3,
				LocoFunctionTypeTimer = 4
			};

			enum LocoFunctionIcon : uint8_t
			{
				// Do not change numbers!
				// Only add numbers!
				// If you add numbers, add them also in ProtocolMaerklin.cpp too
				LocoFunctionIconNone = 0,
				LocoFunctionIconDefault = 1,
				// logical functions
				LocoFunctionIconShuntingMode = 2,
				LocoFunctionIconInertia,
				// light functions
				LocoFunctionIconLight = 32,
				LocoFunctionIconHeadlightLowBeam,
				LocoFunctionIconHeadlightHighBeam,
				LocoFunctionIconHeadlightForward,
				LocoFunctionIconHeadlightReverse,
				LocoFunctionIconBackLightForward,
				LocoFunctionIconBackLightReverse,
				LocoFunctionIconBlinkingLight,
				LocoFunctionIconInteriorLight1,
				LocoFunctionIconInteriorLight2,
				LocoFunctionIconTableLight1,
				LocoFunctionIconTableLight2,
				LocoFunctionIconTableLight3,
				LocoFunctionIconCabLight1,
				LocoFunctionIconCabLight2,
				LocoFunctionIconCabLight12,
				LocoFunctionIconDriversDeskLight,
				LocoFunctionIconTrainDestinationIndicator,
				LocoFunctionIconTrainNumberIndicator,
				LocoFunctionIconEngineLight,
				LocoFunctionIconFireBox,
				LocoFunctionIconStairsLight,
				// mechanical functions
				LocoFunctionIconSmokeGenerator = 64,
				LocoFunctionIconTelex1,
				LocoFunctionIconTelex2,
				LocoFunctionIconTelex12,
				LocoFunctionIconPanto1,
				LocoFunctionIconPanto2,
				LocoFunctionIconPanto12,
				LocoFunctionIconUp,
				LocoFunctionIconDown,
				LocoFunctionIconUpDown1,
				LocoFunctionIconUpDown2,
				LocoFunctionIconLeft,
				LocoFunctionIconRight,
				LocoFunctionIconLeftRight,
				LocoFunctionIconTurnLeft,
				LocoFunctionIconTurnRight,
				LocoFunctionIconCrane,
				LocoFunctionIconMagnet,
				LocoFunctionIconCraneHook,
				LocoFunctionIconFan,
				// sound functions
				LocoFunctionIconNoSound = 96,
				LocoFunctionIconSoundGeneral,
				LocoFunctionIconRunning1,
				LocoFunctionIconRunning2,
				LocoFunctionIconEngine1,
				LocoFunctionIconEngine2,
				LocoFunctionIconBreak1,
				LocoFunctionIconBreak2,
				LocoFunctionIconCurve,
				LocoFunctionIconHorn1,
				LocoFunctionIconHorn2,
				LocoFunctionIconWhistle1,
				LocoFunctionIconWhistle2,
				LocoFunctionIconBell,
				LocoFunctionIconGenerator,
				LocoFunctionIconGearBox,
				LocoFunctionIconGearUp,
				LocoFunctionIconGearDown,
				LocoFunctionIconStationAnnouncement1,
				LocoFunctionIconStationAnnouncement2,
				LocoFunctionIconStationAnnouncement3,
				LocoFunctionIconShovelCoal,
				LocoFunctionIconOpenDoor,
				LocoFunctionIconCloseDoor,
				LocoFunctionIconFan1,
				LocoFunctionIconFan2,
				LocoFunctionIconFan3,
				LocoFunctionIconCompressedAir,
				LocoFunctionIconReliefValve,
				LocoFunctionIconSteamBlowOut,
				LocoFunctionIconSteamBlow,
				LocoFunctionIconDrainValve,
				LocoFunctionIconAirPump,
				LocoFunctionIconWaterPump,
				LocoFunctionIconRailJoint,
				LocoFunctionIconCoupler,
				LocoFunctionIconBufferPush,
				LocoFunctionIconFillWater,
				LocoFunctionIconFillDiesel,
				LocoFunctionIconFillGas,
				LocoFunctionIconSpeak,
				LocoFunctionIconShakingRust,
				LocoFunctionIconSand,
				LocoFunctionIconMusic1,
				LocoFunctionIconMusic2,
				LocoFunctionIconPanto,
				LocoFunctionIconRadio,

				MaxLocoFunctionIcons
			};

			typedef uint8_t LocoFunctionTimer;

			inline LocoFunctions()
			:	count(1),
			 	states{LocoFunctionStateOff},
			 	types{LocoFunctionTypeNone},
			 	icons{LocoFunctionIconNone},
			 	timers{0}
			{
			}

			inline LocoFunctions(const std::string& serialized)
			: LocoFunctions()
			{
				Deserialize(serialized);
			}

			inline void SetFunctionState(const LocoFunctionNr nr, const LocoFunctionState state)
			{
				if (nr >= MaxCount)
				{
					return;
				}
				states[nr] = state;
			}

			inline LocoFunctionState GetFunctionState(const LocoFunctionNr nr) const
			{
				if (nr >= MaxCount)
				{
					return LocoFunctionStateOff;
				}
				LocoFunctionState out = states[nr];
				return out;
			}

			inline std::vector<LocoFunctionState> GetFunctionStates() const
			{
				std::vector<LocoFunctionState> out;
				for (LocoFunctionNr i = 0; i < count; ++i)
				{
					out.push_back(states[i]);
				}
				return out;
			}

			inline void SetNrOfFunctions(const LocoFunctionNr nr)
			{
				// externally we count the functions additional to F0
				// internally we count all the functions including F0
				if (nr + 1 > count)
				{
					for (LocoFunctionNr i = count; i <= nr; ++i)
					{
						states[i] = LocoFunctionStateOff;
						types[i] = LocoFunctionTypePermanent;
						icons[i] = LocoFunctionIconDefault;
					}
				}
				if (nr + 1 < count)
				{
					for (LocoFunctionNr i = nr + 1; i <= count; ++i)
					{
						states[i] = LocoFunctionStateOff;
						types[i] = LocoFunctionTypeNone;
						icons[i] = LocoFunctionIconNone;
					}
				}
				count = nr + 1;
			}

			inline LocoFunctionNr GetNrOfFunctions() const
			{
				return count - 1;
			}

			inline void ConfigureFunction(const LocoFunctionNr nr,
				const LocoFunctionType type,
				const LocoFunctionIcon icon,
				const LocoFunctionTimer timer)
			{
				if (nr > MaxFunctions)
				{
					return;
				}
				if (count == (nr + 1) && type == LocoFunctionTypeNone)
				{
					--count;
				}
				else if (count == nr && type != LocoFunctionTypeNone)
				{
					++count;
				}
				types[nr] = type;
				icons[nr] = icon;
				timers[nr] = (type == LocoFunctionTypeTimer ? timer : 0);
			}

			std::string Serialize() const override;

			bool Deserialize(const std::string& serialized) override;

			static const LocoFunctionNr MaxFunctions = 32;

		private:
			bool DeserializeNew(__attribute__((unused)) const std::string& serialized);

			// FIXME: remove later
			bool DeserializeOld(const std::string& serialized);

			static const LocoFunctionNr MaxCount = MaxFunctions + 1; // f0 - f32 = 33
			LocoFunctionNr count;
			LocoFunctionState states[MaxCount];
			LocoFunctionType types[MaxCount];
			LocoFunctionIcon icons[MaxCount];
			LocoFunctionTimer timers[MaxCount];
	};
} // namespace DataModel

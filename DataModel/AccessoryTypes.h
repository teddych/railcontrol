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

namespace DataModel
{
	enum AccessoryType : unsigned char
	{
		AccessoryTypeDefault = 0,

		SignalTypeSimpleLeft = 0,
		SignalTypeSimpleRight = 1,

		SwitchTypeLeft = 0,
		SwitchTypeRight
	};

	enum AccessoryState : bool
	{
		DefaultState = false,

		AccessoryStateOff = false,
		AccessoryStateOn = true,

		SignalStateRed = false,
		SignalStateGreen = true,

		SwitchStateTurnout = false,
		SwitchStateStraight = true
	};

	typedef unsigned short AccessoryPulseDuration;
	static const AccessoryPulseDuration DefaultAccessoryPulseDuration = 100;
} // namespace DataModel


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

#include <mutex>
#include <pthread.h>
#include <vector>

#include "Manager.h"

class DelayedCallEntry
{
	public:
		DelayedCallEntry(Manager& manager, const controlType_t controlType, const unsigned long waitTime)
		:	waitTime(waitTime),
		 	manager(manager),
		 	controlType(controlType)
		{}
		virtual ~DelayedCallEntry(){}
		virtual void Execute() = 0;
		unsigned long waitTime;

	protected:
		Manager& manager;
		controlType_t controlType;
};

class DelayedCallEntryAccessory : public DelayedCallEntry
{
	public:
		DelayedCallEntryAccessory(Manager& manager, const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const unsigned long timeout)
		:	DelayedCallEntry(manager, controlType, timeout),
		 	accessoryID(accessoryID),
		 	state(state),
		 	inverted(inverted)
		{}

		void Execute() override { manager.AccessoryState(controlType, accessoryID, state, inverted, false); }

	private:
		accessoryID_t accessoryID;
		accessoryState_t state;
		bool inverted;
};

class DelayedCallEntrySwitch : public DelayedCallEntry
{
	public:
		DelayedCallEntrySwitch(Manager& manager, const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const unsigned long timeout)
		:	DelayedCallEntry(manager, controlType, timeout),
		 	switchID(switchID),
		 	state(state),
		 	inverted(inverted)
		{}

		void Execute() override { manager.SwitchState(controlType, switchID, state, inverted, false); }

	private:
		switchID_t switchID;
		switchState_t state;
		bool inverted;
};

class DelayedCallEntrySignal : public DelayedCallEntry
{
	public:
		DelayedCallEntrySignal(Manager& manager, const controlType_t controlType, const signalID_t signalID, const signalState_t state, const bool inverted, const unsigned long timeout)
		:	DelayedCallEntry(manager, controlType, timeout),
		 	signalID(signalID),
		 	state(state),
		 	inverted(inverted)
		{}

		void Execute() override { manager.SignalState(controlType, signalID, state, inverted, false); }

	private:
		signalID_t signalID;
		signalState_t state;
		bool inverted;
};

class DelayedCall
{
	public:
		DelayedCall(Manager& manager);
		~DelayedCall();

		void Worker();
		void Accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const unsigned long timeout);
		void Switch(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const unsigned long timeout);
		void Signal(const controlType_t controlType, const signalID_t signalID, const switchState_t state, const bool inverted, const unsigned long timeout);
		unsigned long counter;

		const unsigned int CountStep = 100; // ms

	private:
		Manager& manager;
		std::mutex mutex;
		std::vector<DelayedCallEntry*> waitingCalls;
		volatile bool run;
		std::thread thread;
};

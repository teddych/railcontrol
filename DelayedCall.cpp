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

#include <unistd.h>

#include "DelayedCall.h"
#include "Utils/Utils.h"

DelayedCall::DelayedCall(Manager& manager)
:	counter(0),
	manager(manager),
 	run(true),
 	thread(&DelayedCall::Worker, this)
{
}

DelayedCall::~DelayedCall()
{
	run = false;
	thread.join();
}

void DelayedCall::Worker()
{
	Utils::Utils::SetThreadName("DelayedCall");
	while(run)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(CountStep));
		std::lock_guard<std::mutex> lock(mutex);
		++counter;
		auto callElement = waitingCalls.begin();
		while (run && callElement != waitingCalls.end())
		{
			DelayedCallEntry* entry = *callElement;
			if (entry->waitTime > counter)
			{
				++callElement;
				continue;
			}
			entry->Execute();
			callElement = waitingCalls.erase(callElement);
			delete entry;
		}
	}

	// cleanup
	std::lock_guard<std::mutex> lock(mutex);
	auto callElement = waitingCalls.begin();
	while (callElement != waitingCalls.end())
	{
		DelayedCallEntry* entry = *callElement;
		entry->Execute();
		callElement = waitingCalls.erase(callElement);
		delete entry;
	}
}

void DelayedCall::Accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const unsigned long waitTime)
{
	unsigned int count = (waitTime / CountStep) + counter + 1;
	auto entry = new DelayedCallEntryAccessory(manager, controlType, accessoryID, state, inverted, count);
	std::lock_guard<std::mutex> lock(mutex);
	waitingCalls.push_back(entry);
}

void DelayedCall::Switch(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const unsigned long waitTime)
{
	unsigned int count = (waitTime / CountStep) + counter + 1;
	auto entry = new DelayedCallEntrySwitch(manager, controlType, switchID, state, inverted, count);
	std::lock_guard<std::mutex> lock(mutex);
	waitingCalls.push_back(entry);
}

void DelayedCall::Signal(const controlType_t controlType, const signalID_t signalID, const signalState_t state, const bool inverted, const unsigned long waitTime)
{
	unsigned int count = (waitTime / CountStep) + counter + 1;
	auto entry = new DelayedCallEntrySignal(manager, controlType, signalID, state, inverted, count);
	std::lock_guard<std::mutex> lock(mutex);
	waitingCalls.push_back(entry);
}

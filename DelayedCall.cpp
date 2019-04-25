#include <unistd.h>

#include "DelayedCall.h"

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
	pthread_setname_np(pthread_self(), "DelayedCall");
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

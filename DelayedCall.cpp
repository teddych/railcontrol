#include <unistd.h>

#include "DelayedCall.h"

DelayedCall::DelayedCall(Manager& manager)
:	counter(0),
	manager(manager),
 	run(true),
	thread(Thread, this)
{
}

DelayedCall::~DelayedCall()
{
	run = false;
	thread.join();
}

void DelayedCall::Thread(DelayedCall* thisClass)
{
	while(thisClass->run)
	{
		usleep(DelayedCall::CountStep);
		std::lock_guard<std::mutex> lock(thisClass->mutex);
		++thisClass->counter;
		auto callElement = thisClass->waitingCalls.begin();
		while (thisClass->run && callElement != thisClass->waitingCalls.end())
		{
			DelayedCallEntry* entry = *callElement;
			if (entry->timeout > thisClass->counter)
			{
				++callElement;
				continue;
			}
			entry->Execute();
			callElement = thisClass->waitingCalls.erase(callElement);
			delete entry;
		}
	}

	// cleanup
	std::lock_guard<std::mutex> lock(thisClass->mutex);
	auto callElement = thisClass->waitingCalls.begin();
	while (callElement != thisClass->waitingCalls.end())
	{
		DelayedCallEntry* entry = *callElement;
		entry->Execute();
		callElement = thisClass->waitingCalls.erase(callElement);
		delete entry;
	}
}

void DelayedCall::Accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const unsigned long timeout)
{
	unsigned long count = (timeout * 1000 / CountStep) + counter + 1;
	auto entry = new DelayedCallEntryAccessory(manager, controlType, accessoryID, state, count);
	std::lock_guard<std::mutex> lock(mutex);
	waitingCalls.push_back(entry);
}

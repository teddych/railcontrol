#pragma once

#include <mutex>
#include <pthread.h>
#include <vector>

#include "manager.h"

class DelayedCallEntry
{
	public:
		DelayedCallEntry(Manager& manager, const controlType_t controlType, const unsigned long timeout)
		:	timeout(timeout),
		 	manager(manager),
		 	controlType(controlType)
		{}
		virtual ~DelayedCallEntry(){}
		virtual void Execute() = 0;
		unsigned long timeout;

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

		void Execute() override { manager.accessory(controlType, accessoryID, state, inverted, false); }

	private:
		accessoryID_t accessoryID;
		accessoryState_t state;
		bool inverted;
};

class DelayedCall
{
	public:
		DelayedCall(Manager& manager);
		~DelayedCall();

		static void Thread(DelayedCall* delayedCall);
		void Accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const unsigned long timeout);
		unsigned long counter;

	private:
		static const unsigned long CountStep = 100000; // us
		//static const unsigned long CountStep = 500000; // us
		Manager& manager;
		std::mutex mutex;
		std::vector<DelayedCallEntry*> waitingCalls;
		volatile bool run;
		std::thread thread;
};

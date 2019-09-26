#pragma once

#include <map>
#include <mutex>

#include "datatypes.h"

namespace DataModel
{
	class LockableItem
	{
		public:
			enum lockState_t : unsigned char
			{
				LockStateFree = 0,
				LockStateReserved,
				LockStateSoftLocked,
				LockStateHardLocked
			};

			LockableItem()
			:	lockState(LockStateFree),
			 	locoID(LocoNone)
			{
			}

			std::string Serialize() const;
			bool Deserialize(const std::map<std::string,std::string> arguments);


			locoID_t GetLoco() const { return locoID; }
			lockState_t GetLockState() const { return lockState; }
			virtual bool Reserve(const locoID_t locoID);
			virtual bool Lock(const locoID_t locoID);
			virtual bool Release(const locoID_t locoID);

			bool IsInUse() const { return lockState != LockStateFree || locoID != LocoNone; }


		private:
			std::mutex lockMutex;
			lockState_t lockState;
			locoID_t locoID;
	};
} // namespace DataModel


#pragma once

#include <map>
#include <mutex>

#include "datatypes.h"

namespace datamodel
{
	class LockableItem
	{
		public:
			LockableItem(const lockState_t lockState)
			:	lockState(lockState),
			 	locoID(LocoNone)
			{
			}

			LockableItem() {}

			std::string Serialize() const;
			bool Deserialize(const std::map<std::string,std::string> arguments);


			locoID_t GetLoco() const { return locoID; }
			lockState_t GetLockState() const { return lockState; }
			bool Reserve(const locoID_t locoID);
			bool Lock(const locoID_t locoID);
			bool Release(const locoID_t locoID);

			bool IsInUse() const { return lockState != LockStateFree || locoID != LocoNone; }


		protected:
			std::mutex updateMutex;

		private:
			lockState_t lockState;
			locoID_t locoID;
	};
} // namespace datamodel


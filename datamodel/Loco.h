#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "datatypes.h"
#include "Logger/Logger.h"
#include "datamodel/HardwareHandle.h"
#include "datamodel/LocoFunctions.h"
#include "datamodel/LocoSlaves.h"
#include "datamodel/object.h"
#include "Utils/ThreadSafeQueue.h"

class Manager;

namespace datamodel
{
	class Street;
	class Track;

	class Loco : public Object, public HardwareHandle
	{
		public:
			enum nrOfTracksToReserve_t : unsigned char
			{
				ReserveOne = 1,
				ReserveTwo = 2
			};

			Loco(Manager* manager, const locoID_t locoID)
			:	Object(locoID),
			 	manager(manager),
				speed(MinSpeed),
				direction(DirectionRight),
				slaves(manager, locoID),
				state(LocoStateManual),
				trackFrom(nullptr),
				trackFirst(nullptr),
				trackSecond(nullptr),
				streetFirst(nullptr),
				streetSecond(nullptr),
				feedbackIdFirst(FeedbackNone),
				feedbackIdReduced(FeedbackNone),
				feedbackIdCreep(FeedbackNone),
				feedbackIdStop(FeedbackNone),
				feedbackIdOver(FeedbackNone),
				feedbackIdsReached(),
				wait(0)
			{
				logger = Logger::Logger::GetLogger("Loco " + name);
				SetNrOfFunctions(0);
			}

			Loco(Manager* manager, const std::string& serialized)
			:	manager(manager),
				speed(MinSpeed),
				direction(DirectionRight),
				slaves(manager),
				state(LocoStateManual),
				trackFrom(nullptr),
				trackFirst(nullptr),
				trackSecond(nullptr),
				streetFirst(nullptr),
				streetSecond(nullptr),
				feedbackIdFirst(FeedbackNone),
				feedbackIdReduced(FeedbackNone),
				feedbackIdCreep(FeedbackNone),
				feedbackIdStop(FeedbackNone),
				feedbackIdOver(FeedbackNone),
				feedbackIdsReached(),
				wait(0)
			{
				Deserialize(serialized);
				logger = Logger::Logger::GetLogger("Loco " + name);
			}

			~Loco();

			objectType_t GetObjectType() const { return ObjectTypeLoco; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;

			virtual void SetName(const std::string& name) override
			{
				Object::SetName(name);
				logger = Logger::Logger::GetLogger("Loco " + name);
			}

			bool GoToAutoMode();
			bool GoToManualMode();

			bool ToTrack(const trackID_t trackID);
			bool Release();
			bool IsRunningFromTrack(const trackID_t trackID) const;

			const char* GetStateText() const;
			void LocationReached(const feedbackID_t feedbackID);

			void Speed(const locoSpeed_t speed);
			locoSpeed_t Speed() const { return speed; }

			void SetFunction(const function_t nr, const bool state) { functions.SetFunction(nr, state); }
			bool GetFunction(const function_t nr) const { return functions.GetFunction(nr); }
			void SetNrOfFunctions(const function_t nr) { functions.SetNrOfFunctions(nr); }
			function_t GetNrOfFunctions() const { return functions.GetNrOfFunctions(); }
			void SetDirection(const direction_t direction) { this->direction = direction; }
			direction_t GetDirection() const { return direction; }

			bool IsInUse() const { return this->speed > 0 || this->state != LocoStateManual || this->trackFrom != nullptr || this->streetFirst != nullptr; }

			bool GetCommuter() const { return commuter; }
			length_t GetLength() const { return length; }
			void SetLength(const length_t length) {  this->length = length; }
			locoSpeed_t GetMaxSpeed() const { return maxSpeed; }
			locoSpeed_t GetTravelSpeed() const { return travelSpeed; }
			locoSpeed_t GetReducedSpeed() const { return reducedSpeed; }
			locoSpeed_t GetCreepSpeed() const { return creepSpeed; }
			void SetCommuter(bool commuter) { this->commuter = commuter; }
			void SetMaxSpeed(locoSpeed_t speed) { maxSpeed = speed; }
			void SetTravelSpeed(locoSpeed_t speed) { travelSpeed = speed; }
			void SetReducedSpeed(locoSpeed_t speed) { reducedSpeed = speed; }
			void SetCreepSpeed(locoSpeed_t speed) { creepSpeed = speed; }

			void SetSlave(const locoID_t slaveID, const LocoMasterSlave::speedRelation_t speedRelation);
			void DeleteSlave(const locoID_t slaveID);
			void GetSlaveSpeedRelation(const locoID_t slaveID);

		private:
			void SetMinThreadPriorityAndThreadName();
			void AutoMode();
			void SearchDestinationFirst();
			void SearchDestinationSecond();
			datamodel::Street* SearchDestination(datamodel::Track* oldToTrack, const bool allowLocoTurn);
			void FeedbackIdFirstReached();
			void FeedbackIdStopReached();

			enum locoState_t : unsigned char
			{
				LocoStateManual = 0,
				LocoStateOff,
				LocoStateSearchingFirst,
				LocoStateSearchingSecond,
				LocoStateRunning,
				LocoStateStopping,
				LocoStateError
			};

			Manager* manager;
			mutable std::mutex stateMutex;
			std::thread locoThread;

			length_t length;
			bool commuter;
			locoSpeed_t maxSpeed;
			locoSpeed_t travelSpeed;
			locoSpeed_t reducedSpeed;
			locoSpeed_t creepSpeed;

			locoSpeed_t speed;
			direction_t direction;

			LocoSlaves slaves;

			volatile locoState_t state;
			Track* trackFrom;
			Track* trackFirst;
			Track* trackSecond;
			Street* streetFirst;
			Street* streetSecond;
			volatile feedbackID_t feedbackIdFirst;
			volatile feedbackID_t feedbackIdReduced;
			volatile feedbackID_t feedbackIdCreep;
			volatile feedbackID_t feedbackIdStop;
			volatile feedbackID_t feedbackIdOver;
			Utils::ThreadSafeQueue<feedbackID_t> feedbackIdsReached;
			wait_t wait;

			LocoFunctions functions;

			Logger::Logger* logger;
	};
} // namespace datamodel

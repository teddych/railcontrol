#pragma once

#include <mutex>
#include <string>
#include <thread>

#include "datatypes.h"
#include "Logger/Logger.h"
#include "datamodel/HardwareHandle.h"
#include "datamodel/object.h"

class Manager;

namespace datamodel
{
	class Street;
	class Track;
	class LocoFunctions
	{
		public:
			LocoFunctions()
			:	count(1),
			 	states{0}
			{
			}

			LocoFunctions(const std::string& serialized)
			: LocoFunctions()
			{
				Deserialize(serialized);
			}

			void SetFunction(const function_t nr, const bool state)
			{
				if (nr >= maxCount)
				{
					return;
				}
				states[nr] = state;
			}

			bool GetFunction(const function_t nr) const
			{
				if (nr >= maxCount)
				{
					return false;
				}
				bool out = states[nr];
				return out;
			}

			void SetNrOfFunctions(const function_t nr)
			{
				// externally we count the functions additional to F0
				// internally we count all the functions including F0
				count = nr + 1;
			}

			function_t GetNrOfFunctions() const
			{
				return count - 1;
			}

			std::string Serialize() const
			{
				std::string out;
				for (function_t i = 0; i < count; ++i)
				{
					out += (states[i] ? "1" : "0");
				}
				return out;
			}

			void Deserialize(const std::string& serialized)
			{
				count = serialized.size();
				if (count > maxCount)
				{
					count = maxCount;
				}
				for (function_t i = 0; i < count; ++i)
				{
					states[i] = serialized[i] == '1';
				}

			}

			static const function_t maxFunctions = 32;

		private:
			static const function_t maxCount = maxFunctions - 1; // f0 - f32 = 33
			function_t count;
			bool states[maxCount];
	};

	class Loco : public Object, public HardwareHandle
	{
		public:
			Loco(Manager* manager, const locoID_t locoID)
			:	Object(locoID),
			 	manager(manager),
				speed(MinSpeed),
				direction(DirectionRight),
				state(LocoStateManual),
				trackIdFrom(TrackNone),
				trackIdFirst(TrackNone),
				trackIdSecond(TrackNone),
				streetIdFirst(StreetNone),
				streetIdSecond(StreetNone),
				feedbackIdFirst(FeedbackNone),
				feedbackIdReduced(FeedbackNone),
				feedbackIdCreep(FeedbackNone),
				feedbackIdStop(FeedbackNone),
				feedbackIdOver(FeedbackNone)
			{
				logger = Logger::Logger::GetLogger("Loco " + name);
				SetNrOfFunctions(0);
			}

			Loco(Manager* manager, const std::string& serialized)
			:	manager(manager),
				speed(MinSpeed),
				direction(DirectionRight),
				state(LocoStateManual),
				trackIdFrom(TrackNone),
				trackIdFirst(TrackNone),
				trackIdSecond(TrackNone),
				streetIdFirst(StreetNone),
				streetIdSecond(StreetNone),
				feedbackIdFirst(FeedbackNone),
				feedbackIdReduced(FeedbackNone),
				feedbackIdCreep(FeedbackNone),
				feedbackIdStop(FeedbackNone),
				feedbackIdOver(FeedbackNone)
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

			bool Start();
			bool Stop();

			bool ToTrack(const trackID_t trackID);
			bool Release();
			trackID_t GetTrack() const { return trackIdFirst; }
			streetID_t GetStreetFirst() const { return streetIdFirst; }
			streetID_t GetStreetSecond() const { return streetIdSecond; }
			const char* const GetStateText() const;
			void LocationReached(const feedbackID_t feedbackID);

			void Speed(const locoSpeed_t speed) { this->speed = speed; }
			const locoSpeed_t Speed() const { return speed; }

			void SetFunction(const function_t nr, const bool state) { functions.SetFunction(nr, state); }
			bool GetFunction(const function_t nr) const { return functions.GetFunction(nr); }
			void SetNrOfFunctions(const function_t nr) { functions.SetNrOfFunctions(nr); }
			function_t GetNrOfFunctions() const { return functions.GetNrOfFunctions(); }
			void SetDirection(const direction_t direction) { this->direction = direction; }
			direction_t GetDirection() const { return direction; }

			bool IsInUse() const { return this->speed > 0 || this->state != LocoStateManual || this->trackIdFirst != TrackNone || this->streetIdFirst != StreetNone; }

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

		private:
			void AutoMode();
			void SearchDestinationFirst();
			void SearchDestinationSecond();
			datamodel::Street* SearchDestination(datamodel::Track* oldToTrack, const bool allowLocoTurn);
			void SetMinThreadPriority();

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
			std::mutex stateMutex;
			std::thread locoThread;

			length_t length;
			bool commuter;
			locoSpeed_t maxSpeed;
			locoSpeed_t travelSpeed;
			locoSpeed_t reducedSpeed;
			locoSpeed_t creepSpeed;

			locoSpeed_t speed;
			direction_t direction;

			volatile locoState_t state;
			volatile trackID_t trackIdFrom;
			volatile trackID_t trackIdFirst;
			volatile trackID_t trackIdSecond;
			volatile streetID_t streetIdFirst;
			volatile streetID_t streetIdSecond;
			volatile feedbackID_t feedbackIdFirst;
			volatile feedbackID_t feedbackIdReduced;
			volatile feedbackID_t feedbackIdCreep;
			volatile feedbackID_t feedbackIdStop;
			volatile feedbackID_t feedbackIdOver;

			LocoFunctions functions;

			Logger::Logger* logger;
	};
} // namespace datamodel

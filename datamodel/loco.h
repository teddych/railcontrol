#pragma once

#include <mutex>
#include <string>
#include <thread>

#include "datatypes.h"
#include "Logger/Logger.h"
#include "datamodel/object.h"

class Manager;

namespace datamodel
{
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

	class Loco : public Object
	{
		public:
			Loco(Manager* manager,
				const locoID_t locoID,
				const std::string& name,
				const controlID_t controlID,
				const protocol_t protocol,
				const address_t address,
				const function_t nr)
			:	Object(locoID, name),
			 	controlID(controlID),
				protocol(protocol),
				address(address),
				manager(manager),
				speed(0),
				state(LocoStateManual),
				trackID(TrackNone),
				streetID(StreetNone),
				direction(DirectionLeft)
			{
				logger = Logger::Logger::GetLogger("Loco " + name);
			}

			Loco(Manager* manager, const std::string& serialized)
			:	manager(manager),
				speed(0),
				state(LocoStateManual),
				streetID(StreetNone)
			{
				Deserialize(serialized);
				logger = Logger::Logger::GetLogger("Loco " + name);
			}

			~Loco();

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;

			bool Start();
			bool Stop();

			bool ToTrack(const trackID_t trackID);
			bool ToTrack(const trackID_t trackIDOld, const trackID_t trackIDNew);
			bool Release();
			trackID_t GetTrack() const { return trackID; }
			streetID_t GetStreet() const { return streetID; }
			const char* const GetStateText() const;
			void DestinationReached();

			void Speed(const locoSpeed_t speed) { this->speed = speed; }
			const locoSpeed_t Speed() const { return speed; }

			void SetFunction(const function_t nr, const bool state) { functions.SetFunction(nr, state); }
			bool GetFunction(const function_t nr) const { return functions.GetFunction(nr); }
			void SetNrOfFunctions(const function_t nr) { functions.SetNrOfFunctions(nr); }
			function_t GetNrOfFunctions() const { return functions.GetNrOfFunctions(); }
			void SetDirection(const direction_t direction) { this->direction = direction; }
			direction_t GetDirection() const { return direction; }

			bool IsInUse() const { return this->speed > 0 || this->state != LocoStateManual || this->trackID != TrackNone || this->streetID != StreetNone; }

			// FIXME: make private:
			controlID_t controlID;
			protocol_t protocol;
			address_t address;

		private:
			void AutoMode(Loco* loco);

			enum locoState_t : unsigned char
			{
				LocoStateManual = 0,
				LocoStateOff,
				LocoStateSearching,
				LocoStateRunning,
				LocoStateStopping,
				LocoStateError
			};

			Manager* manager;
			locoSpeed_t speed;
			locoState_t state;
			trackID_t trackID;
			streetID_t streetID;
			std::mutex stateMutex;
			std::thread locoThread;

			LocoFunctions functions;
			direction_t direction;

			Logger::Logger* logger;
	};
} // namespace datamodel

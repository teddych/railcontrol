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

			bool start();
			bool stop();

			bool toTrack(const trackID_t trackID);
			bool toTrack(const trackID_t trackIDOld, const trackID_t trackIDNew);
			bool release();
			trackID_t track() const { return trackID; }
			streetID_t street() const { return streetID; }
			const char* const getStateText() const;
			void destinationReached();

			void Speed(const LocoSpeed speed) { this->speed = speed; }
			const LocoSpeed Speed() const { return speed; }

			void SetFunction(const function_t nr, const bool state) { functions.SetFunction(nr, state); }
			bool GetFunction(const function_t nr) const { return functions.GetFunction(nr); }
			void SetNrOfFunctions(const function_t nr) { functions.SetNrOfFunctions(nr); }
			function_t GetNrOfFunctions() const { return functions.GetNrOfFunctions(); }
			void SetDirection(const direction_t direction) { this->direction = direction; }
			direction_t GetDirection() const { return direction; }

			bool isInUse() const;

			// FIXME: make private:
			controlID_t controlID;
			protocol_t protocol;
			address_t address;

		private:
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
			LocoSpeed speed;
			locoState_t state;
			trackID_t trackID;
			streetID_t streetID;
			std::mutex stateMutex;
			std::thread locoThread;

			LocoFunctions functions;
			direction_t direction;

			Logger::Logger* logger;

			void autoMode(Loco* loco);
	};

	inline bool Loco::isInUse() const
	{
		return this->speed > 0 || this->state != LocoStateManual || this->trackID != TrackNone || this->streetID != StreetNone;
	}

} // namespace datamodel

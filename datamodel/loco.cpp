
#include <map>
#include <sstream>

#include "datamodel/loco.h"
#include "datamodel/track.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;
using std::vector;

namespace datamodel
{
	Loco::~Loco()
	{
		while (true)
		{
			{
				std::lock_guard<std::mutex> Guard(stateMutex);
				if (state == LocoStateManual)
				{
					return;
				}
			}
			logger->Info("Waiting until {0} has stopped", name);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	std::string Loco::Serialize() const
	{
		stringstream ss;
		ss << "objectType=Loco;" << Object::Serialize()
			<< ";controlID=" << static_cast<int>(controlID)
			<< ";protocol=" << static_cast<int>(protocol)
			<< ";address=" << static_cast<int>(address)
			<< ";functions=" << functions.Serialize()
			<< ";direction=" << (direction == DirectionRight ? "right" : "left")
			<< ";trackID=" << static_cast<int>(toTrackID)
			<< ";maxspeed=" << maxSpeed
			<< ";travelspeed=" << travelSpeed
			<< ";reducedspeed=" << reducedSpeed
			<< ";creepspeed=" << creepSpeed;
		return ss.str();
	}

	bool Loco::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		Object::Deserialize(arguments);
		if (!arguments.count("objectType") || arguments.at("objectType").compare("Loco") != 0)
		{
			return false;
		}
		controlID = GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address = GetIntegerMapEntry(arguments, "address", AddressNone);
		toTrackID = GetIntegerMapEntry(arguments, "trackID", TrackNone);
		functions.Deserialize(GetStringMapEntry(arguments, "functions", "0"));
		direction = (GetStringMapEntry(arguments, "direction", "right").compare("right") == 0 ? DirectionRight : DirectionLeft);
		maxSpeed = GetIntegerMapEntry(arguments, "maxspeed", MaxSpeed);
		travelSpeed = GetIntegerMapEntry(arguments, "travelspeed", DefaultTravelSpeed);
		reducedSpeed = GetIntegerMapEntry(arguments, "reducedspeed", DefaultReducedSpeed);
		creepSpeed = GetIntegerMapEntry(arguments, "creepspeed", DefaultCreepSpeed);
		return true;
	}

	bool Loco::ToTrack(const trackID_t trackID)
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		// there must not be set a track
		if (this->toTrackID != TrackNone)
		{
			return false;
		}
		this->toTrackID = trackID;
		return true;
	}

	bool Loco::Release()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (state != LocoStateManual)
		{
			state = LocoStateOff;
		}
		fromTrackID = TrackNone;
		toTrackID = TrackNone;
		streetID = StreetNone;
		feedbackIdStop = FeedbackNone;
		return true;
	}

	bool Loco::Start()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (toTrackID == TrackNone)
		{
			logger->Warning("Can not start {0} because it is not in a track", name);
			return false;
		}
		if (state == LocoStateError)
		{
			logger->Warning("Can not start {0} because it is in error state", name);
			return false;
		}
		if (state == LocoStateOff)
		{
			locoThread.join();
			state = LocoStateManual;
		}
		if (state != LocoStateManual)
		{
			logger->Info("Can not start {0} because it is already running", name);
			return false;
		}

		state = LocoStateSearching;
		locoThread = std::thread(&datamodel::Loco::AutoMode, this, this);

		return true;
	}

	bool Loco::Stop()
	{
		{
			std::lock_guard<std::mutex> Guard(stateMutex);
			switch (state)
			{
				case LocoStateManual:
					manager->LocoSpeed(ControlTypeInternal, objectID, 0);
					return true;

				case LocoStateOff:
					case LocoStateSearching:
					case LocoStateError:
					state = LocoStateOff;
					break;

				case LocoStateRunning:
				case LocoStateStopping:
					logger->Info("{0} is actually running, waiting until reached its destination", name);
					state = LocoStateStopping;
					return false;

				default:
					logger->Error("{0} is in unknown state. Setting to error state and setting speed to 0.", name);
					state = LocoStateError;
					manager->LocoSpeed(ControlTypeInternal, objectID, 0);
					return false;
			}
		}
		locoThread.join();
		state = LocoStateManual;
		return true;
	}

	void Loco::AutoMode(Loco* loco)
	{
		const string& name = loco->name;
		logger->Info("{0} is now in automode", name);

		sched_param param;
		int policy;
		pthread_t self = pthread_self();
		pthread_getschedparam(self, &policy, &param);
		param.__sched_priority = sched_get_priority_min(policy);
		pthread_setschedparam(self, policy, &param);

		while (true)
		{
			{
				std::lock_guard<std::mutex> Guard(loco->stateMutex);
				switch (loco->state)
				{
					case LocoStateOff:
						// automode is turned off, terminate thread
						logger->Info("{0} is now in manual mode", name);
						return;

					case LocoStateSearching:
						loco->SearchDestination();
						break;

					case LocoStateRunning:
						// loco is already running, waiting until destination reached
						break;

					case LocoStateStopping:
						logger->Info("{0} has not yet reached its destination. Going to manual mode when it reached its destination.", name);
						break;

					case LocoStateManual:
						logger->Error("{0} is in manual state while automode is running. Putting loco into error state", name);
						state = LocoStateError;
						// fall through / no break

					case LocoStateError:
						logger->Error("{0} is in error state.", name);
						manager->LocoSpeed(ControlTypeInternal, objectID, 0);
						break;
				}
			}
			// FIXME: make configurable
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	void Loco::SearchDestination()
	{
		// check if already running
		if (streetID != StreetNone)
		{
			state = LocoStateError;
			logger->Error("{0} has already a street reserved. Going to error state.", name);
			return;
		}

		// get possible destinations
		Track* toTrack = manager->GetTrack(toTrackID);
		if (toTrack == nullptr)
		{
			state = LocoStateOff;
			logger->Info("{0} is not on a track. Switching to manual mode.", name);
			return;
		}

		if (toTrack->objectID != toTrackID)
		{
			state = LocoStateError;
			logger->Error("{0} thinks it is on track {1} but there is {2}. Going to error state.", name, toTrack->Name(), manager->GetLocoName(toTrack->GetLoco()));
			return;
		}
		logger->Info("Looking for new destination starting from {0}.", toTrack->Name());

		// FIXME: get best fitting destination and reserve street
		vector<Street*> streets;
		toTrack->ValidStreets(streets);
		for (auto street : streets)
		{
			if (street->Reserve(objectID) == false)
			{
				continue;
			}
			if (street->Lock(objectID) == false)
			{
				street->Release(objectID);
				continue;
			}

			if (street->Execute() == false)
			{
				street->Release(objectID);
				continue;
			}
			fromTrackID = toTrackID;
			toTrackID = street->DestinationTrack();
			streetID = street->objectID;
			feedbackIdStop = street->feedbackIdStop;
			logger->Info("Heading to {0} via {1}", manager->GetTrackName(toTrackID), street->name);
			break;
		}

		if (streetID == StreetNone)
		{
			logger->Info("No valid street found for {0}", name);
			return;
		}

		// start loco
		manager->TrackPublishState(toTrackID);
		// FIXME: make maxspeed configurable
		manager->LocoSpeed(ControlTypeInternal, objectID, travelSpeed);
		state = LocoStateRunning;
	}

	void Loco::DestinationReached(const feedbackID_t feedbackID)
	{
		if (feedbackID == feedbackIdStop)
		{
			locoID_t& locoID = objectID;
			manager->LocoSpeed(ControlTypeInternal, locoID, MinSpeed);
			std::lock_guard<std::mutex> Guard(stateMutex);
			// set loco to new track
			Street* oldStreet = manager->GetStreet(streetID);
			Track* fromTrack = manager->GetTrack(fromTrackID);
			if (oldStreet == nullptr || fromTrack == nullptr)
			{
				Speed(MinSpeed);
				state = LocoStateError;
				logger->Error("{0} is running in automode without a street / track. Putting loco into error state", name);
				return;
			}

			fromTrackID = oldStreet->DestinationTrack();
			manager->LocoDestinationReached(locoID, streetID, fromTrackID);
			oldStreet->Release(locoID);
			fromTrack->Release(locoID);
			fromTrackID = TrackNone;

			streetID = StreetNone;
			feedbackIdStop = FeedbackNone;
			// set state
			state = (state == LocoStateRunning /* else is LocoStateStopping */? LocoStateSearching : LocoStateOff);
			logger->Info("{0} reached its destination", name);
			manager->TrackPublishState(fromTrack);
		}
	}

	const char* const Loco::GetStateText() const
	{
		switch (state)
		{
			case LocoStateManual:
				return "manual";

			case LocoStateOff:
				return "off";

			case LocoStateSearching:
				return "searching";

			case LocoStateRunning:
				return "running";

			case LocoStateStopping:
				return "stopping";

			case LocoStateError:
				return "error";

			default:
				return "unknown";
		}
	}
} // namespace datamodel


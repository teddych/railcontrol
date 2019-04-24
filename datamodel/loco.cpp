#include <algorithm>
#include <map>
#include <sstream>

#include "datamodel/loco.h"
#include "datamodel/track.h"
#include "manager.h"
#include "util.h"

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
		ss << "objectType=Loco"
			<< ";" << Object::Serialize()
			<< ";" << HardwareHandle::Serialize()
			<< ";functions=" << functions.Serialize()
			<< ";direction=" << (direction == DirectionRight ? "right" : "left")
			<< ";trackID=" << static_cast<int>(trackIdFrom)
			<< ";length=" << length
			<< ";commuter=" << static_cast<int>(commuter)
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
		HardwareHandle::Deserialize(arguments);
		trackIdFrom = GetIntegerMapEntry(arguments, "trackID", TrackNone);
		functions.Deserialize(GetStringMapEntry(arguments, "functions", "0"));
		direction = (GetStringMapEntry(arguments, "direction", "right").compare("right") == 0 ? DirectionRight : DirectionLeft);
		length = static_cast<length_t>(GetIntegerMapEntry(arguments, "length", 0));
		commuter = GetBoolMapEntry(arguments, "commuter", false);
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
		if (this->trackIdFrom != TrackNone)
		{
			return false;
		}
		this->trackIdFrom = trackID;
		return true;
	}

	bool Loco::Release()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (state != LocoStateManual)
		{
			state = LocoStateOff;
		}

		Street* street = manager->GetStreet(streetIdFirst);
		if (street != nullptr)
		{
			street->Release(objectID);
		}
		street = manager->GetStreet(streetIdSecond);
		if (street != nullptr)
		{
			street->Release(objectID);
		}
		Track* track = manager->GetTrack(trackIdFrom);
		if (track != nullptr)
		{
			track->Release(objectID);
		}
		track = manager->GetTrack(trackIdFirst);
		if (track != nullptr)
		{
			track->Release(objectID);
		}
		track = manager->GetTrack(trackIdSecond);
		if (track != nullptr)
		{
			track->Release(objectID);
		}
		trackIdFrom = TrackNone;
		trackIdFirst = TrackNone;
		trackIdSecond = TrackNone;
		streetIdFirst = StreetNone;
		streetIdSecond = StreetNone;
		feedbackIdOver = FeedbackNone;
		feedbackIdStop = FeedbackNone;
		feedbackIdCreep = FeedbackNone;
		feedbackIdReduced = FeedbackNone;
		feedbackIdFirst = FeedbackNone;
		return true;
	}

	bool Loco::Start()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (trackIdFrom == TrackNone)
		{
			logger->Warning("Can not start {0} because it is not on a track", name);
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

		state = LocoStateSearchingFirst;
		locoThread = std::thread(&datamodel::Loco::AutoMode, this);

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
				case LocoStateSearchingFirst:
				case LocoStateSearchingSecond:
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

	void Loco::SetMinThreadPriority()
	{
		sched_param param;
		int policy;
		pthread_t self = pthread_self();
		pthread_getschedparam(self, &policy, &param);
		param.__sched_priority = sched_get_priority_min(policy);
		pthread_setschedparam(self, policy, &param);
	}

	void Loco::AutoMode()
	{
		SetMinThreadPriority();
		const string& name = GetName();
		logger->Info("{0} is now in automode", name);

		while (true)
		{
			{ // sleep is outside the lock
				std::lock_guard<std::mutex> Guard(stateMutex);
				switch (state)
				{
					case LocoStateOff:
						// automode is turned off, terminate thread
						logger->Info("{0} is now in manual mode", name);
						return;

					case LocoStateSearchingFirst:
						SearchDestinationFirst();
						break;

					case LocoStateSearchingSecond:
						SearchDestinationSecond();
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

	void Loco::SearchDestinationFirst()
	{
		// check if already running
		if (streetIdFirst != StreetNone || streetIdSecond != StreetNone)
		{
			state = LocoStateError;
			logger->Error("{0} has already a street reserved. Going to error state.", name);
			return;
		}

		Track* oldTrack = manager->GetTrack(trackIdFrom); // FIXME: save trackpointer because of performance
		if (oldTrack == nullptr)
		{
			state = LocoStateOff;
			logger->Info("{0} is not on a track. Switching to manual mode.", name);
			return;
		}

		if (oldTrack->GetID() != trackIdFrom)
		{
			state = LocoStateError;
			logger->Error("{0} thinks it is on track {1} but there is {2}. Going to error state.", name, oldTrack->GetName(), manager->GetLocoName(oldTrack->GetLoco()));
			return;
		}
		logger->Debug("Looking for new destination starting from {0}.", oldTrack->GetName());

		Street* usedStreet = SearchDestination(oldTrack, true);
		if (usedStreet == nullptr)
		{
			logger->Debug("No valid street found for {0}", name);
			return;
		}


		trackID_t newTrackIdFirst = usedStreet->GetToTrack();
		Track* newTrack = manager->GetTrack(newTrackIdFirst);
		if (newTrack == nullptr)
		{
			return;
		}

		trackIdFirst = newTrackIdFirst;
		streetIdFirst = usedStreet->GetID();
		feedbackIdFirst = FeedbackNone;
		feedbackIdReduced = usedStreet->GetFeedbackIdReduced();
		feedbackIdCreep = usedStreet->GetFeedbackIdCreep();
		feedbackIdStop = usedStreet->GetFeedbackIdStop();
		feedbackIdOver = usedStreet->GetFeedbackIdOver();
		bool turnLoco = (oldTrack->GetLocoDirection() != usedStreet->GetFromDirection());
		direction_t newLocoDirection = static_cast<direction_t>(direction != turnLoco);
		if (turnLoco)
		{
			oldTrack->SetLocoDirection(usedStreet->GetFromDirection());
			manager->TrackPublishState(oldTrack);
		}
		manager->LocoDirection(ControlTypeInternal, this, newLocoDirection);
		newTrack->SetLocoDirection(static_cast<direction_t>(!usedStreet->GetToDirection()));
		logger->Info("Heading to {0} via {1}", newTrack->GetName(), usedStreet->GetName());

		// start loco
		manager->TrackPublishState(newTrack);
		manager->LocoSpeed(ControlTypeInternal, objectID, travelSpeed);
		state = LocoStateSearchingSecond;
	}

	void Loco::SearchDestinationSecond()
	{
		// check if already running
		if (streetIdSecond != StreetNone)
		{
			state = LocoStateError;
			logger->Error("{0} has already a street reserved. Going to error state.", name);
			return;
		}

		Track* oldTrack = manager->GetTrack(trackIdFirst); // FIXME: save trackpointer because of performance
		if (oldTrack == nullptr)
		{
			state = LocoStateOff;
			logger->Info("{0} is not on a track. Switching to manual mode.", name);
			return;
		}

		if (oldTrack->GetID() != trackIdFirst)
		{
			state = LocoStateError;
			logger->Error("{0} thinks it is on track {1} but there is {2}. Going to error state.", name, oldTrack->GetName(), manager->GetLocoName(oldTrack->GetLoco()));
			return;
		}
		logger->Debug("Looking for new destination starting from {0}.", oldTrack->GetName());

		Street* usedStreet = SearchDestination(oldTrack, false);
		if (usedStreet == nullptr)
		{
			logger->Debug("No valid street found for {0}", name);
			return;
		}
		// FIXME: replace with code in SearchDestination
		bool turnLoco = (oldTrack->GetLocoDirection() != usedStreet->GetFromDirection());
		if (turnLoco)
		{
			return;
		}

		trackID_t newTrackIdSecond = usedStreet->GetToTrack();
		Track* newTrack = manager->GetTrack(newTrackIdSecond);
		if (newTrack == nullptr)
		{
			return;
		}

		trackIdSecond = newTrackIdSecond;
		streetIdSecond = usedStreet->GetID();
		feedbackIdFirst = feedbackIdStop;
		feedbackIdOver = usedStreet->GetFeedbackIdOver();
		feedbackIdStop = usedStreet->GetFeedbackIdStop();
		feedbackIdCreep = usedStreet->GetFeedbackIdCreep();
		feedbackIdReduced = usedStreet->GetFeedbackIdReduced();
		newTrack->SetLocoDirection(static_cast<direction_t>(!usedStreet->GetToDirection()));
		logger->Info("Heading to {0} via {1}", newTrack->GetName(), usedStreet->GetName());

		// start loco
		manager->TrackPublishState(newTrack);
		manager->LocoSpeed(ControlTypeInternal, objectID, travelSpeed);
		state = LocoStateRunning;
	}

	Street* Loco::SearchDestination(Track* track, const bool allowLocoTurn)
	{
		vector<Street*> validStreets;
		track->GetValidStreets(this, allowLocoTurn, validStreets);
		for (auto street : validStreets)
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
			return street;
		}
		return nullptr;
	}

	void Loco::LocationReached(const feedbackID_t feedbackID)
	{
		locoID_t& locoID = objectID;
		if (feedbackID == feedbackIdOver)
		{
			manager->LocoSpeed(ControlTypeInternal, this, MinSpeed);
			manager->Booster(ControlTypeInternal, BoosterStop);
			logger->Error("{0} hit overrun feedback {1}", name, manager->GetFeedbackName(feedbackID));
			return;
		}

		if (feedbackID == feedbackIdStop)
		{
			manager->LocoSpeed(ControlTypeInternal, this, MinSpeed);
			std::lock_guard<std::mutex> Guard(stateMutex);
			// set loco to new track
			Street* oldStreet = manager->GetStreet(streetIdFirst);
			Track* fromTrack = manager->GetTrack(trackIdFrom);
			if (oldStreet == nullptr || fromTrack == nullptr)
			{
				Speed(MinSpeed);
				state = LocoStateError;
				logger->Error("{0} is running in automode without a street / track. Putting loco into error state", name);
				return;
			}

			trackIdFrom = oldStreet->GetToTrack();
			manager->LocoDestinationReached(locoID, streetIdFirst, trackIdFrom);
			oldStreet->Release(locoID);
			fromTrack->Release(locoID);

			streetIdFirst = StreetNone;

			feedbackIdStop = FeedbackNone;
			feedbackIdCreep = FeedbackNone;
			feedbackIdReduced = FeedbackNone;
			logger->Info("{0} reached its destination", name);
			// set state
			switch (state)
			{
				case LocoStateSearchingSecond:
					state = LocoStateSearchingFirst;
					return;

				case LocoStateStopping:
					state = LocoStateOff;
					return;

				default:
					state = LocoStateError;
					logger->Error("{0} is running in impossible automode state. Putting loco into error state", name);
					return;
			}
		}

		if (feedbackID == feedbackIdCreep)
		{
			if (speed > creepSpeed)
			{
				manager->LocoSpeed(ControlTypeInternal, this, creepSpeed);
			}
			return;
		}

		if (feedbackID == feedbackIdReduced)
		{
			if (speed > reducedSpeed)
			{
				manager->LocoSpeed(ControlTypeInternal, this, reducedSpeed);
			}
			return;
		}

		if (feedbackID == feedbackIdFirst)
		{
			std::lock_guard<std::mutex> Guard(stateMutex);
			// set loco to new track
			Street* oldStreet = manager->GetStreet(streetIdFirst);
			Track* oldFromTrack = manager->GetTrack(trackIdFrom);
			if (oldStreet == nullptr || oldFromTrack == nullptr)
			{
				Speed(MinSpeed);
				state = LocoStateError;
				logger->Error("{0} is running in automode without a street / track. Putting loco into error state", name);
				return;
			}

			oldStreet->Release(locoID);
			oldFromTrack->Release(locoID);
			trackIdFrom = trackIdFirst;
			trackIdFirst = trackIdSecond;

			streetIdFirst = streetIdSecond;
			streetIdSecond = StreetNone;

			feedbackIdFirst = FeedbackNone;

			// set state
			switch (state)
			{
				case LocoStateRunning:
					state = LocoStateSearchingSecond;
					return;

				case LocoStateStopping:
					// do nothing
					return;

				default:
					state = LocoStateError;
					logger->Error("{0} is running in impossible automode state. Putting loco into error state", name);
					return;
			}
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

			case LocoStateSearchingFirst:
			case LocoStateSearchingSecond:
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


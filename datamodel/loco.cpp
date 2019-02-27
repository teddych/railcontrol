
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
			logger->Info("Waiting until loco {0} has stopped", name);
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
			<< ";trackID=" << static_cast<int>(trackID);
		return ss.str();
	}

	bool Loco::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		Object::Deserialize(arguments);
		if (!arguments.count("objectType") || arguments.at("objectType").compare("Loco") != 0)
		{
			return false;
		}
		controlID = GetIntegerMapEntry(arguments, "controlID", ControlIdNone);
		protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address = GetIntegerMapEntry(arguments, "address", AddressNone);
		trackID = GetIntegerMapEntry(arguments, "trackID", TrackNone);
		functions.Deserialize(GetStringMapEntry(arguments, "functions", "0"));
		direction = (GetStringMapEntry(arguments, "direction", "right").compare("right") == 0 ? DirectionRight : DirectionLeft);
		return true;
	}

	bool Loco::toTrack(const trackID_t trackID)
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		// there must not be set a track
		if (this->trackID != TrackNone)
		{
			return false;
		}
		this->trackID = trackID;
		return true;
	}

	bool Loco::toTrack(const trackID_t trackIDOld, const trackID_t trackIDNew)
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		// the old track must be the currently set track
		if (trackID != trackIDOld)
		{
			return false;
		}
		trackID = trackIDNew;
		return true;
	}

	bool Loco::release()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (state != LocoStateManual)
		{
			state = LocoStateOff;
		}
		trackID = TrackNone;
		streetID = StreetNone;
		return true;
	}

	bool Loco::start()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (trackID == TrackNone)
		{
			logger->Warning("Can not start loco {0} because it is not in a track", name);
			return false;
		}
		if (state == LocoStateError)
		{
			logger->Warning("Can not start loco {0} because it is in error state", name);
			return false;
		}
		if (state == LocoStateOff)
		{
			locoThread.join();
			state = LocoStateManual;
		}
		if (state != LocoStateManual)
		{
			logger->Info("Can not start loco {0} because it is already running", name);
			return false;
		}

		state = LocoStateSearching;
		locoThread = std::thread(&datamodel::Loco::autoMode, this, this);

		return true;
	}

	bool Loco::stop()
	{
		{
			std::lock_guard<std::mutex> Guard(stateMutex);
			switch (state)
			{
				case LocoStateManual:
					manager->locoSpeed(ControlTypeInternal, objectID, 0);
					return true;

				case LocoStateOff:
					case LocoStateSearching:
					case LocoStateError:
					state = LocoStateOff;
					break;

				case LocoStateRunning:
				case LocoStateStopping:
					logger->Info("Loco {0} is actually running, waiting until loco reached its destination", name);
					state = LocoStateStopping;
					return false;

				default:
					logger->Error("Loco {0} is in unknown state. Setting to error state and setting speed to 0.", name);
					state = LocoStateError;
					manager->locoSpeed(ControlTypeInternal, objectID, 0);
					return false;
			}
		}
		locoThread.join();
		state = LocoStateManual;
		return true;
	}

	void Loco::autoMode(Loco* loco)
	{
		const string& name = loco->name;
		logger->Info("Loco {0} is now in automode", name);
		while (true)
		{
			{
				std::lock_guard<std::mutex> Guard(loco->stateMutex);
				switch (loco->state)
				{
					case LocoStateOff:
						// automode is turned off, terminate thread
						logger->Info("Loco {0} is now in manual mode", name);
						return;
					case LocoStateSearching:
					{
						logger->Info("Looking for new track for loco {0}", name);
						// check if already running
						if (streetID != StreetNone)
						{
							loco->state = LocoStateError;
							logger->Error("Loco {0} has already a street reserved. Going to error state.", name);
							break;
						}
						// get possible destinations
						Track* fromTrack = manager->getTrack(trackID);
						if (!fromTrack)
						{
							break;
						}
						// get best fitting destination and reserve street
						vector<Street*> streets;
						fromTrack->getValidStreets(streets);
						trackID_t toTrackID = TrackNone;
						for (auto street : streets)
						{
							if (street->reserve(objectID))
							{
								street->lock(objectID);
								streetID = street->objectID;
								toTrackID = street->destinationTrack();
								logger->Info("Loco \"{0}\" found street \"{1}\" with destination \"{2}\"", name, street->name, manager->getTrackName(toTrackID));
								break; // break for
							}
						}

						if (streetID == StreetNone)
						{
							logger->Info("No valid street found for loco {0}", name);
							break; // break switch
						}

						// start loco
						manager->locoStreet(objectID, streetID, toTrackID);
						// FIXME: make maxspeed configurable
						manager->locoSpeed(ControlTypeInternal, objectID, MaxSpeed >> 1);
						loco->state = LocoStateRunning;
						break;
					}

					case LocoStateRunning:
						// loco is already running, waiting until destination reached
						break;

					case LocoStateStopping:
						logger->Info("Loco {0} has not yet reached its destination. Going to manual mode when it reached its destination.", name);
						break;

					case LocoStateManual:
						logger->Error("Loco {0} is in manual state while automode is running. Putting loco into error state", name);
						state = LocoStateError;

					case LocoStateError:
						logger->Error("Loco {0} is in error state.", name);
						manager->locoSpeed(ControlTypeInternal, objectID, 0);
						break;
				}
			}
			// FIXME: make configurable
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	const char* const Loco::getStateText() const
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

	void Loco::destinationReached()
	{
		std::lock_guard<std::mutex> Guard(stateMutex);
		manager->locoSpeed(ControlTypeInternal, objectID, 0);
		// set loco to new track
		Street* street = manager->getStreet(streetID);
		if (street == nullptr)
		{
			state = LocoStateError;
			logger->Error("Loco {0} is running in automode without a street. Putting loco into error state", name);
			return;
		}
		trackID = street->destinationTrack();
		manager->locoDestinationReached(objectID, streetID, trackID);
		// release old track & old street
		street->release(objectID);
		streetID = StreetNone;
		// set state
		if (state == LocoStateRunning)
		{
			state = LocoStateSearching;
		}
		else
		{ // LOCO_STATE_STOPPING
			state = LocoStateOff;
		}
		logger->Info("Loco {0} reached its destination", name);
	}

} // namespace datamodel


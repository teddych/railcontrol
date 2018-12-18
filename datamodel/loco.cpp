
#include <datamodel/track.h>
#include <map>
#include <sstream>
#include <unistd.h>

#include "datamodel/loco.h"
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
			xlog("Waiting until loco %s has stopped", name.c_str());
			sleep(1);
		}
	}

	std::string Loco::serialize() const
	{
		stringstream ss;
		ss << "objectType=Loco;" << Object::serialize()
			<< ";controlID=" << static_cast<int>(controlID)
			<< ";protocol=" << static_cast<int>(protocol)
			<< ";address=" << static_cast<int>(address)
			<< ";functions=" << functions.Serialize()
			<< ";direction=" << (direction == DirectionRight ? "right" : "left")
			<< ";trackID=" << static_cast<int>(trackID);
		return ss.str();
	}

	bool Loco::deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		Object::deserialize(arguments);
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
			xlog("Can not start loco %s because it is not in a track", name.c_str());
			return false;
		}
		if (state == LocoStateError)
		{
			xlog("Can not start loco %s because it is in error state", name.c_str());
			return false;
		}
		if (state == LocoStateOff)
		{
			locoThread.join();
			state = LocoStateManual;
		}
		if (state != LocoStateManual)
		{
			xlog("Can not start loco %s because it is already running", name.c_str());
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
					manager->locoSpeed(ControlTypeAutomode, objectID, 0);
					return true;

				case LocoStateOff:
					case LocoStateSearching:
					case LocoStateError:
					state = LocoStateOff;
					break;

				case LocoStateRunning:
					case LocoStateStopping:
					xlog("Loco %s is actually running, waiting until loco reached its destination", name.c_str());
					state = LocoStateStopping;
					return false;

				default:
					xlog("Loco %s is in unknown state. Setting to error state and setting speed to 0.", name.c_str());
					state = LocoStateError;
					manager->locoSpeed(ControlTypeAutomode, objectID, 0);
					return false;
			}
		}
		locoThread.join();
		state = LocoStateManual;
		return true;
	}

	void Loco::autoMode(Loco* loco)
	{
		const char* name = loco->name.c_str();
		xlog("Loco %s is now in automode", name);
		while (true)
		{
			{
				std::lock_guard<std::mutex> Guard(loco->stateMutex);
				switch (loco->state)
				{
					case LocoStateOff:
						// automode is turned off, terminate thread
						xlog("Loco %s is now in manual mode", name);
						return;
					case LocoStateSearching:
						{
						xlog("Looking for new track for loco %s", name);
						// check if already running
						if (streetID != StreetNone)
						{
							loco->state = LocoStateError;
							xlog("Loco %s has already a street reserved. Going to error state.", name);
							break;
						}
						// get possible destinations
						Track* fromTrack = manager->getTrack(trackID);
						if (!fromTrack)
							break;
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
								xlog("Loco \"%s\" found street \"%s\" with destination \"%s\"", name, street->name.c_str(), manager->getTrackName(toTrackID).c_str());
								break; // break for
							}
						}

						if (streetID == StreetNone)
						{
							xlog("No valid street found for loco %s", name);
							break; // break switch
						}

						// start loco
						manager->locoStreet(objectID, streetID, toTrackID);
						// FIXME: make maxspeed configurable
						manager->locoSpeed(ControlTypeAutomode, objectID, MaxSpeed >> 1);
						loco->state = LocoStateRunning;
						break;
					}
					case LocoStateRunning:
						// loco is already running, waiting until destination reached
						break;
					case LocoStateStopping:
						xlog("Loco %s has not yet reached its destination. Going to manual mode when it reached its destination.", name);
						break;
					case LocoStateManual:
						xlog("Loco %s is in manual state while automode is running. Putting loco into error state", name);
						state = LocoStateError;
					case LocoStateError:
						xlog("Loco %s is in error state.", name);
						manager->locoSpeed(ControlTypeAutomode, objectID, 0);
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
		manager->locoSpeed(ControlTypeAutomode, objectID, 0);
		// set loco to new track
		Street* street = manager->getStreet(streetID);
		if (street == nullptr)
		{
			state = LocoStateError;
			xlog("Loco %s is running in automode without a street. Putting loco into error state", name.c_str());
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
		xlog("Loco %s reached its destination", name.c_str());
	}

} // namespace datamodel


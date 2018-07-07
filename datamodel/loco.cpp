
#include <map>
#include <sstream>
#include <unistd.h>

#include "block.h"
#include "loco.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;
using std::vector;

namespace datamodel {

	Loco::Loco(Manager* manager, const locoID_t locoID, const std::string& name, const controlID_t controlID, const protocol_t protocol, const address_t address) :
		Object(locoID, name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		manager(manager),
		speed(0),
		state(LocoStateManual),
		blockID(BlockNone),
		streetID(StreetNone) {
	}

	Loco::Loco(Manager* manager, const std::string& serialized) :
		manager(manager),
		speed(0),
		state(LocoStateManual),
		streetID(StreetNone) {
		deserialize(serialized);
	}

	Loco::~Loco() {
		while(true) {
			{
				std::lock_guard<std::mutex> Guard(stateMutex);
				if (state == LocoStateManual) {
					return;
				}
			}
			xlog("Waiting until loco %s has stopped", name.c_str());
			sleep(1);
		}
	}

	std::string Loco::serialize() const {
		stringstream ss;
		ss << "objectType=Loco;" << Object::serialize() << ";controlID=" << (int)controlID << ";protocol=" << (int)protocol << ";address=" << (int)address << ";blockID=" << (int)blockID;
		return ss.str();
	}

	bool Loco::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		Object::deserialize(arguments);
		if (!arguments.count("objectType") || arguments.at("objectType").compare("Loco") != 0) {
			return false;
		}
		if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
		if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
		if (arguments.count("address")) address = stoi(arguments.at("address"));
		if (arguments.count("blockID")) blockID = stoi(arguments.at("blockID"));
		return true;
	}

	bool Loco::toBlock(const blockID_t blockID) {
		std::lock_guard<std::mutex> Guard(stateMutex);
		// there must not be set a block
		if (this->blockID != BlockNone) {
			return false;
		}
		this->blockID = blockID;
		return true;
	}

	bool Loco::toBlock(const blockID_t blockIDOld, const blockID_t blockIDNew) {
		std::lock_guard<std::mutex> Guard(stateMutex);
		// the old block must be the currently set block
		if (blockID != blockIDOld) {
			return false;
		}
		blockID = blockIDNew;
		return true;
	}

	bool Loco::release() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (state != LocoStateManual) {
			state = LocoStateOff;
		}
		blockID = BlockNone;
		streetID = StreetNone;
		return true;
	}

	bool Loco::start() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (blockID == BlockNone) {
			xlog("Can not start loco %s because it is not in a block", name.c_str());
			return false;
		}
		if (state == LocoStateError) {
			xlog("Can not start loco %s because it is in error state", name.c_str());
			return false;
		}
		if (state == LocoStateOff) {
			locoThread.join();
			state = LocoStateManual;
		}
		if (state != LocoStateManual) {
			xlog("Can not start loco %s because it is already running", name.c_str());
			return false;
		}

		state = LocoStateSearching;
		locoThread = std::thread(&datamodel::Loco::autoMode, this, this);

		return true;
	}

	bool Loco::stop() {
		{
			std::lock_guard<std::mutex> Guard(stateMutex);
			switch (state) {
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

	void Loco::autoMode(Loco* loco) {
		const char* name = loco->name.c_str();
		xlog("Loco %s is now in automode", name);
		while (true) {
			{
				std::lock_guard<std::mutex> Guard(loco->stateMutex);
				switch (loco->state) {
					case LocoStateOff:
						// automode is turned off, terminate thread
						xlog("Loco %s is now in manual mode", name);
						return;
					case LocoStateSearching:
					{
						xlog("Looking for new Block for loco %s", name);
						// check if already running
						if (streetID != StreetNone) {
							loco->state = LocoStateError;
							xlog("Loco %s has already a street reserved. Going to error state.", name);
							break;
						}
						// get possible destinations
						Block* fromBlock = manager->getBlock(blockID);
						if (!fromBlock) break;
						// get best fitting destination and reserve street
						vector<Street*> streets;
						fromBlock->getValidStreets(streets);
						blockID_t toBlockID = BlockNone;
						for (auto street : streets) {
							if (street->reserve(objectID)) {
								street->lock(objectID);
								streetID = street->objectID;
								toBlockID = street->destinationBlock();
								xlog("Loco \"%s\" found street \"%s\" with destination \"%s\"", name, street->name.c_str(), manager->getBlockName(toBlockID).c_str());
								break; // break for
							}
						}

						if (streetID == StreetNone) {
							xlog("No valid street found for loco %s", name);
							break; // break switch
						}

						// start loco
						manager->locoStreet(objectID, streetID, toBlockID);
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
			usleep(1000000);
		}
	}

	void Loco::destinationReached() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		manager->locoSpeed(ControlTypeAutomode, objectID, 0);
		// set loco to new block
		Street* street = manager->getStreet(streetID);
		if (street == nullptr) {
			state = LocoStateError;
			xlog("Loco %s is running in automode without a street. Putting loco into error state", name.c_str());
			return;
		}
		blockID = street->destinationBlock();
		manager->locoDestinationReached(objectID, streetID, blockID);
		// release old block & old street
		street->release(objectID);
		streetID = StreetNone;
		// set state
		if (state == LocoStateRunning) {
			state = LocoStateSearching;
		}
		else { // LOCO_STATE_STOPPING
			state = LocoStateOff;
		}
		xlog("Loco %s reached its destination", name.c_str());
	}

} // namespace datamodel


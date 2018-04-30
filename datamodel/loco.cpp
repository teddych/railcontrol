
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
		state(LOCO_STATE_MANUAL),
		blockID(BLOCK_NONE),
		streetID(STREET_NONE) {
	}

	Loco::Loco(Manager* manager, const std::string& serialized) :
		manager(manager),
		speed(0),
		state(LOCO_STATE_MANUAL),
		streetID(STREET_NONE) {
		deserialize(serialized);
	}

	Loco::~Loco() {
		while(true) {
			{
				std::lock_guard<std::mutex> Guard(stateMutex);
				if (state == LOCO_STATE_MANUAL) {
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
		if (this->blockID != BLOCK_NONE) {
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
		if (state != LOCO_STATE_MANUAL) {
			state = LOCO_STATE_OFF;
		}
		blockID = BLOCK_NONE;
		streetID = STREET_NONE;
		return true;
	}

	bool Loco::start() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (blockID == BLOCK_NONE) {
			xlog("Can not start loco %s because it is not in a block", name.c_str());
			return false;
		}
		if (state == LOCO_STATE_ERROR) {
			xlog("Can not start loco %s because it is in error state", name.c_str());
			return false;
		}
		if (state == LOCO_STATE_OFF) {
			locoThread.join();
			state = LOCO_STATE_MANUAL;
		}
		if (state != LOCO_STATE_MANUAL) {
			xlog("Can not start loco %s because it is already running", name.c_str());
			return false;
		}

		state = LOCO_STATE_SEARCHING;
		locoThread = std::thread(&datamodel::Loco::autoMode, this, this);

		return true;
	}

	bool Loco::stop() {
		{
			std::lock_guard<std::mutex> Guard(stateMutex);
			switch (state) {
				case LOCO_STATE_MANUAL:
					manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, 0);
					return true;

				case LOCO_STATE_OFF:
				case LOCO_STATE_SEARCHING:
				case LOCO_STATE_ERROR:
					state = LOCO_STATE_OFF;
					break;

				case LOCO_STATE_RUNNING:
				case LOCO_STATE_STOPPING:
					xlog("Loco %s is actually running, waiting until loco reached its destination", name.c_str());
					state = LOCO_STATE_STOPPING;
					return false;

				default:
					xlog("Loco %s is in unknown state. Setting to error state and setting speed to 0.", name.c_str());
					state = LOCO_STATE_ERROR;
					manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, 0);
					return false;
			}
		}
		locoThread.join();
		state = LOCO_STATE_MANUAL;
		return true;
	}

	void Loco::autoMode(Loco* loco) {
		const char* name = loco->name.c_str();
		xlog("Loco %s is now in automode", name);
		while (true) {
			{
				std::lock_guard<std::mutex> Guard(loco->stateMutex);
				switch (loco->state) {
					case LOCO_STATE_OFF:
						// automode is turned off, terminate thread
						xlog("Loco %s is now in manual mode", name);
						return;
					case LOCO_STATE_SEARCHING:
					{
						xlog("Looking for new Block for loco %s", name);
						// check if already running
						if (streetID != STREET_NONE) {
							loco->state = LOCO_STATE_ERROR;
							xlog("Loco %s has already a street reserved. Going to error state.", name);
							break;
						}
						// get possible destinations
						Block* fromBlock = manager->getBlock(blockID);
						if (!fromBlock) break;
						// get best fitting destination and reserve street
						vector<Street*> streets;
						fromBlock->getValidStreets(streets);
						blockID_t toBlockID = BLOCK_NONE;
						for (auto street : streets) {
							if (street->reserve(objectID)) {
								street->lock(objectID);
								streetID = street->objectID;
								toBlockID = street->destinationBlock();
								xlog("Loco \"%s\" found street \"%s\" with destination \"%s\"", name, street->name.c_str(), manager->getBlockName(toBlockID).c_str());
								break; // break for
							}
						}

						if (streetID == STREET_NONE) {
							xlog("No valid street found for loco %s", name);
							break; // break switch
						}

						// start loco
						manager->locoStreet(objectID, streetID, toBlockID);
						// FIXME: make maxspeed configurable
						manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, MAX_SPEED >> 1);
						loco->state = LOCO_STATE_RUNNING;
						break;
					}
					case LOCO_STATE_RUNNING:
						// loco is already running, waiting until destination reached
						break;
					case LOCO_STATE_STOPPING:
						xlog("Loco %s has not yet reached its destination. Going to manual mode when it reached its destination.", name);
						break;
					case LOCO_STATE_MANUAL:
						xlog("Loco %s is in manual state while automode is running. Putting loco into error state", name);
						state = LOCO_STATE_ERROR;
					case LOCO_STATE_ERROR:
						xlog("Loco %s is in error state.", name);
						manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, 0);
						break;
				}
			}
			// FIXME: make configurable
			usleep(1000000);
		}
	}

	void Loco::destinationReached() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, 0);
		// set loco to new block
		Street* street = manager->getStreet(streetID);
		if (street == nullptr) {
			state = LOCO_STATE_ERROR;
			xlog("Loco %s is running in automode without a street. Putting loco into error state", name.c_str());
			return;
		}
		blockID = street->destinationBlock();
		manager->locoDestinationReached(objectID, streetID, blockID);
		// release old block & old street
		street->release(objectID);
		streetID = STREET_NONE;
		// set state
		if (state == LOCO_STATE_RUNNING) {
			state = LOCO_STATE_SEARCHING;
		}
		else { // LOCO_STATE_STOPPING
			state = LOCO_STATE_OFF;
		}
		xlog("Loco %s reached its destination", name.c_str());
	}

} // namespace datamodel


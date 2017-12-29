
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
		street(NULL),
		//speed(0),
		state(LOCO_STATE_NEVER),
		blockID(BLOCK_NONE) {
	}

	Loco::Loco(Manager* manager, const std::string& serialized) :
		manager(manager),
		street(NULL),
		//speed(0),
		state(LOCO_STATE_NEVER) {
		deserialize(serialized);
	}

	Loco::~Loco() {
		while(true) {
			{
				std::lock_guard<std::mutex> Guard(stateMutex);
				if (state == LOCO_STATE_OFF) {
					locoThread.join();
					return;
				}
				if (state == LOCO_STATE_NEVER) {
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
		if (arguments.count("objectType") && arguments.at("objectType").compare("Loco") == 0) {
			if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
			if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
			if (arguments.count("address")) address = stoi(arguments.at("address"));
			if (arguments.count("blockID")) blockID = stoi(arguments.at("blockID"));
			return true;
		}
		return false;
	}

	bool Loco::start() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (!blockID) {
			stringstream ss;
			ss << "Can not start loco " << name << " because it is not in a block";
			string s(ss.str());
			xlog(s.c_str());
			return false;
		}
		if (state) {
			stringstream ss;
			ss << "Can not start loco " << name << " because it is already running";
			string s(ss.str());
			xlog(s.c_str());
			return false;
		}
		locoThread = std::thread(&datamodel::Loco::autoMode, this, this);

		return true;
	}

	bool Loco::stop() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (state == LOCO_STATE_SEARCHING) {
			state = LOCO_STATE_OFF;
			return true;
		}
		if (state == LOCO_STATE_RUNNING) {
			state = LOCO_STATE_STOPPING;
			return true;
		}
		return false;
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

	bool Loco::releaseBlock() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (!blockID) {
			return false;
		}
		blockID = BLOCK_NONE;
		return true;
	}

	void Loco::autoMode(Loco* loco) {
		stringstream ss;
		ss << "Starting loco " << loco->name;
		string s(ss.str());
		xlog(s.c_str());
		{
			std::lock_guard<std::mutex> Guard(loco->stateMutex);
			if (loco->state == LOCO_STATE_OFF || loco->state == LOCO_STATE_NEVER) loco->state = LOCO_STATE_SEARCHING;
		}

		while (true) {
			{
				std::lock_guard<std::mutex> Guard(loco->stateMutex);
				xlog("Locostate for loco %s: %i", loco->name.c_str(), loco->state);
				switch (loco->state) {
					case LOCO_STATE_NEVER:
					case LOCO_STATE_OFF:
						// automode is turned off
						xlog("Loco stopped %s", loco->name.c_str());
						return;
					case LOCO_STATE_SEARCHING: {
						xlog("Looking for new Block for loco %s", loco->name.c_str());
						// get possible destinations
						Block* fromBlock = manager->getBlock(blockID);
						if (!fromBlock) break;
						// get best fitting destination and reserve street
						vector<Street*> streets;
						fromBlock->getValidStreets(streets);
						for (auto street : streets) {
							if (street->reserve(objectID)) {
								street->lock(objectID);
								this->street = street;
								xlog("Found Street \"%s\" for loco %s", street->name.c_str(), loco->name.c_str());
								break; // break for
							}
						}

						if (!this->street) {
							xlog("No valid street found for loco %s", loco->name.c_str());
							break; // break switch
						}

						// start loco
						manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, 1024);
						loco->state = LOCO_STATE_RUNNING;
						break;
					}
					case LOCO_STATE_RUNNING:
						// loco is already running
						//xlog("Loco is still running");
						break;
					case LOCO_STATE_STOPPING:
						// loco is running but we do not search any more
						loco->state = LOCO_STATE_OFF;
						break;
				}
			}
			usleep(1000000);
		}
	}

	void Loco::destinationReached() {
		xlog("Loco reached its destination");
		std::lock_guard<std::mutex> Guard(stateMutex);
		manager->locoSpeed(MANAGER_ID_AUTOMODE, objectID, 0);
		// set loco to new block
		blockID = street->destinationBlock();
		// release old block
		street->release(objectID);
		// set state
		state = LOCO_STATE_SEARCHING;
	}

} // namespace datamodel


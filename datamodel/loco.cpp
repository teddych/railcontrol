
#include <map>
#include <sstream>
#include <unistd.h>

#include "block.h"
#include "loco.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Loco::Loco(const locoID_t locoID, const std::string& name, const controlID_t controlID, const protocol_t protocol, const address_t address) :
		Object(locoID, name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		speed(0),
		state(LOCO_STATE_OFF) {
	}

	Loco::Loco(const std::string& serialized) :
		speed(0),
		state(LOCO_STATE_OFF) {
		deserialize(serialized);
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
		locoThread = std::thread([this] { autoMode(); });
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
		if (this->blockID == LOCO_NONE) {
			this->blockID = blockID;
			return true;
		}
		return false;
	}

	bool Loco::toBlock(const blockID_t blockIDOld, const blockID_t blockIDNew) {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (blockID == blockIDOld) {
			blockID = blockIDNew;
			return true;
		}
		return false;
	}

	bool Loco::releaseBlock() {
		std::lock_guard<std::mutex> Guard(stateMutex);
		if (blockID) {
			blockID = LOCO_NONE;
			return true;
		}
		return false;
	}

	void Loco::autoMode() {
		stringstream ss;
		ss << "Starting loco " << name;
		string s(ss.str());
		xlog(s.c_str());
		{
			std::lock_guard<std::mutex> Guard(stateMutex);
			if (state == LOCO_STATE_OFF) state = LOCO_STATE_SEARCHING;
		}

		while (true) {
			{
				std::lock_guard<std::mutex> Guard(stateMutex);
				switch (state) {
					case LOCO_STATE_OFF:
						// automode is turned off
						return;
					case LOCO_STATE_SEARCHING:
						// get possible destinations
						//Block* block = manager.getBlock(blockID);
						// get best fitting destination
						// reserve street
						// start loco
						break;
					case LOCO_STATE_RUNNING:
						// loco is already running
						break;
					case LOCO_STATE_STOPPING:
						// loco is running but we do not search any more
						break;
				}
			}
			usleep(100000);
		}
	}

} // namespace datamodel


#include <dlfcn.h>              // dl*
#include <sstream>
#include <string>
#include <vector>

#include "util.h"
#include "storage_handler.h"

using datamodel::Accessory;
using datamodel::Block;
using datamodel::Feedback;
using datamodel::Loco;
using datamodel::Switch;
using std::map;
using std::string;
using std::vector;

namespace storage {

	StorageHandler::StorageHandler(const StorageParams& params) :
		createStorage(NULL),
		destroyStorage(NULL),
		instance(NULL),
		dlhandle(NULL) {

		// generate symbol and library names
		char* error;
		std::stringstream ss;
		ss << "storage/" << params.module << ".so";
		dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
		if (!dlhandle) {
			xlog("Can not open storage library: %s", dlerror());
			return;
		}

		// look for symbol create_*
		ss.str(std::string());
		ss << "create_" << params.module;
		const char* s = ss.str().c_str();
		createStorage_t* newCreateStorage = (createStorage_t*) dlsym(dlhandle, s);
		error = dlerror();
		if (error) {
			xlog("Unable to find symbol %s", s);
			return;
		}

		// look for symbol destroy_*
		ss.str(std::string());
		ss << "destroy_" << params.module;
		s = ss.str().c_str();
		destroyStorage_t* newDestroyStorage = (destroyStorage_t*) dlsym(dlhandle,
		    ss.str().c_str());
		error = dlerror();
		if (error) {
			xlog("Unable to find symbol %s", s);
			return;
		}

		// register  valid symbols
		createStorage = newCreateStorage;
		destroyStorage = newDestroyStorage;

		// start storage
		if (createStorage) {
			instance = createStorage(params);
		}
	}

	StorageHandler::~StorageHandler() {
		// stop storage
		if (instance) {
			destroyStorage(instance);
			instance = NULL;
		}
		// close library
		if (dlhandle) {
			dlclose(dlhandle);
			dlhandle = NULL;
		}
	}

	void StorageHandler::hardwareParams(const hardware::HardwareParams& hardwareParams) {
		if (instance) {
			instance->hardwareParams(hardwareParams);
		}
	}

	void StorageHandler::allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams) {
		if (instance) {
			instance->allHardwareParams(hardwareParams);
		}
	}

	void StorageHandler::loco(const Loco& loco) {
		if (instance) {
			string serialized = loco.serialize();
			instance->saveObject(OBJECT_TYPE_LOCO, loco.locoID, loco.name, serialized);
		}
	}

	void StorageHandler::allLocos(map<locoID_t,datamodel::Loco*>& locos) {
		if (instance) {
			vector<string> objects;
			instance->objectsOfType(OBJECT_TYPE_LOCO, objects);
			for(auto object : objects) {
				Loco* loco = new Loco(object);
				locos[loco->locoID] = loco;
			}
		}
	}

	void StorageHandler::accessory(const Accessory& accessory) {
		if (instance) {
			string serialized = accessory.serialize();
			instance->saveObject(OBJECT_TYPE_ACCESSORY, accessory.accessoryID, accessory.name, serialized);
		}
	}

	void StorageHandler::allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories) {
		if (instance) {
			vector<string> objects;
			instance->objectsOfType(OBJECT_TYPE_ACCESSORY, objects);
			for(auto object : objects) {
				Accessory* accessory = new Accessory(object);
				accessories[accessory->accessoryID] = accessory;
			}
		}
	}

	void StorageHandler::feedback(const Feedback& feedback) {
		if (instance) {
			string serialized = feedback.serialize();
			instance->saveObject(OBJECT_TYPE_FEEDBACK, feedback.feedbackID, feedback.name, serialized);
		}
	}

	void StorageHandler::allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks) {
		if (instance) {
			vector<string> objects;
			instance->objectsOfType(OBJECT_TYPE_FEEDBACK, objects);
			for(auto object : objects) {
				Feedback* feedback = new Feedback(object);
				feedbacks[feedback->feedbackID] = feedback;
			}
		}
	}

	void StorageHandler::block(const Block& block) {
		if (instance) {
			string serialized = block.serialize();
			instance->saveObject(OBJECT_TYPE_BLOCK, block.blockID, block.name, serialized);
		}
	}

	void StorageHandler::allBlocks(std::map<blockID_t,datamodel::Block*>& blocks) {
		if (instance) {
			vector<string> objects;
			instance->objectsOfType(OBJECT_TYPE_BLOCK, objects);
			for(auto object : objects) {
				Block* block = new Block(object);
				blocks[block->blockID] = block;
			}
		}
	}

	void StorageHandler::saveSwitch(const Switch& mySwitch) {
		if (instance) {
			string serialized = mySwitch.serialize();
			instance->saveObject(OBJECT_TYPE_SWITCH, mySwitch.switchID, mySwitch.name, serialized);
		}
	}

	void StorageHandler::allSwitches(std::map<switchID_t,datamodel::Switch*>& switches) {
		if (instance) {
			vector<string> objects;
			instance->objectsOfType(OBJECT_TYPE_SWITCH, objects);
			for(auto object : objects) {
				Switch* mySwitch = new Switch(object);
				switches[mySwitch->switchID] = mySwitch;
			}
		}
	}

} // namespace storage


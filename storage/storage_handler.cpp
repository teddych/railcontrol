#include <dlfcn.h>              // dl*
#include <sstream>
#include <string>
#include <vector>

#include "util.h"
#include "storage/storage_handler.h"

using datamodel::Accessory;
using datamodel::Block;
using datamodel::Feedback;
using datamodel::Loco;
using datamodel::Street;
using datamodel::Switch;
using std::map;
using std::string;
using std::vector;

namespace storage {

	StorageHandler::StorageHandler(Manager* manager, const StorageParams& params) :
		manager(manager),
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
		if (!instance) {
			return;
		}
		instance->hardwareParams(hardwareParams);
	}

	void StorageHandler::allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams) {
		if (!instance) {
			return;
		}
		instance->allHardwareParams(hardwareParams);
	}

	void StorageHandler::deleteHardwareParams(const controlID_t controlID) {
		if (!instance) {
			return;
		}
		instance->deleteHardwareParams(controlID);
	}

	void StorageHandler::loco(const Loco& loco) {
		if (!instance) {
			return;
		}
		string serialized = loco.serialize();
		instance->saveObject(ObjectTypeLoco, loco.objectID, loco.name, serialized);
	}

	void StorageHandler::allLocos(map<locoID_t,datamodel::Loco*>& locos) {
		if (!instance) {
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeLoco, objects);
		for(auto object : objects) {
			Loco* loco = new Loco(manager, object);
			locos[loco->objectID] = loco;
		}
	}

	void StorageHandler::deleteLoco(const locoID_t locoID) {
		if (!instance) {
			return;
		}
		instance->deleteObject(ObjectTypeLoco, locoID);
	}

	void StorageHandler::accessory(const Accessory& accessory) {
		if (!instance) {
			return;
		}
		string serialized = accessory.serialize();
		instance->saveObject(ObjectTypeAccessory, accessory.objectID, accessory.name, serialized);
	}

	void StorageHandler::allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories) {
		if (!instance) {
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeAccessory, objects);
		for(auto object : objects) {
			Accessory* accessory = new Accessory(object);
			accessories[accessory->objectID] = accessory;
		}
	}

	void StorageHandler::deleteAccessory(const accessoryID_t accessoryID) {
		if (!instance) {
			return;
		}
		instance->deleteObject(ObjectTypeAccessory, accessoryID);
	}

	void StorageHandler::feedback(const Feedback& feedback) {
		if (!instance) {
			return;
		}
		string serialized = feedback.serialize();
		instance->saveObject(ObjectTypeFeedback, feedback.objectID, feedback.name, serialized);
	}

	void StorageHandler::allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks) {
		if (!instance) {
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeFeedback, objects);
		for(auto object : objects) {
			Feedback* feedback = new Feedback(manager, object);
			feedbacks[feedback->objectID] = feedback;
		}
	}

	void StorageHandler::deleteFeedback(const feedbackID_t feedbackID) {
		if (!instance) {
			return;
		}
		instance->deleteObject(ObjectTypeFeedback, feedbackID);
	}

	void StorageHandler::block(const Block& block) {
		if (!instance) {
			return;
		}
		string serialized = block.serialize();
		instance->saveObject(ObjectTypeBlock, block.objectID, block.name, serialized);
	}

	void StorageHandler::allBlocks(std::map<blockID_t,datamodel::Block*>& blocks) {
		if (!instance) {
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeBlock, objects);
		for(auto object : objects) {
			Block* block = new Block(object);
			blocks[block->objectID] = block;
		}
	}

	void StorageHandler::deleteBlock(const blockID_t blockID) {
		if (!instance) {
			return;
		}
		instance->deleteObject(ObjectTypeBlock, blockID);
	}

	void StorageHandler::saveSwitch(const Switch& mySwitch) {
		if (!instance) {
			return;
		}
		string serialized = mySwitch.serialize();
		instance->saveObject(ObjectTypeSwitch, mySwitch.objectID, mySwitch.name, serialized);
	}

	void StorageHandler::allSwitches(std::map<switchID_t,datamodel::Switch*>& switches) {
		if (!instance) {
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeSwitch, objects);
		for(auto object : objects) {
			Switch* mySwitch = new Switch(object);
			switches[mySwitch->objectID] = mySwitch;
		}
	}

	void StorageHandler::deleteSwitch(const switchID_t switchID) {
		if (!instance) {
			return;
		}
		instance->deleteObject(ObjectTypeSwitch, switchID);
	}

	void StorageHandler::street(const datamodel::Street& street) {
		if (!instance) {
			return;
		}
		string serialized = street.serialize();
		instance->saveObject(ObjectTypeStreet, street.objectID, street.name, serialized);
	}

	void StorageHandler::allStreets(std::map<streetID_t,datamodel::Street*>& streets) {
		if (!instance) {
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeStreet, objects);
		for(auto object : objects) {
			Street* street = new Street(manager, object);
			streets[street->objectID] = street;
		}
	}

	void StorageHandler::deleteStreet(const streetID_t streetID) {
		if (!instance) {
			return;
		}
		instance->deleteObject(ObjectTypeStreet, streetID);
	}

} // namespace storage


#include <dlfcn.h>              // dl*
#include <sstream>

#include "../util.h"
#include "storage_handler.h"

using datamodel::Loco;
using std::map;

namespace storage {

	StorageHandler::StorageHandler(const StorageParams& params) :
		createStorage(NULL),
		destroyStorage(NULL),
		instance(NULL),
		dlhandle(NULL),
		params(params) {

		// generate symbol and library names
		char* error;
		std::stringstream ss;
		ss << "storage/" << params.name << ".so";

		dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
		if (!dlhandle) {
			xlog("Can not open storage library: %s", dlerror());
			return;
		}
		xlog("Storage engine %s loaded", params.name.c_str());

		// look for symbol create_*
		ss.str(std::string());
		ss << "create_" << params.name;
		const char* s = ss.str().c_str();
		createStorage_t* newCreateStorage = (createStorage_t*) dlsym(dlhandle, s);
		error = dlerror();
		if (error) {
			xlog("Unable to find symbol %s", s);
			return;
		}

		// look for symbol destroy_*
		ss.str(std::string());
		ss << "destroy_" << params.name;
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
		xlog("Storage engine %s unloaded", params.name.c_str());
	}

	void StorageHandler::loco(const Loco& loco) {
		if (instance) {
			instance->loco(loco);
		}
	}

	void StorageHandler::allLocos(map<locoID_t,datamodel::Loco*>& locos) {
		if (instance) {
			return instance->allLocos(locos);
		}
	}

} // namespace storage


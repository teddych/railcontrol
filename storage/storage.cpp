#include "storage.h"

using datamodel::Loco;
using std::vector;

namespace storage {

Storage::Storage(StorageParams& params) {
	createStorage(NULL),
	destroyStorage(NULL),
	instance(NULL),
	dlhandle(NULL),
	params(params) {

  // generate symbol and library names
  char* error;
	std::stringstream ss;
	ss << "storage/" << params.symbol << ".so";
  xlog("Trying to load storage engine %s", params.symbol.c_str());

  dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
  if (!dlhandle) {
    xlog("Can not open storage library: %s", dlerror());
		return;
  }

	// look for symbol create_*
  ss.str(std::string());
	ss << "create_" << symbol;
	const char* s = ss.str().c_str();
  create_storage_t* new_create_storage = (create_storage_t*)dlsym(dlhandle, s);
  error = dlerror();
  if (error) {
    xlog("Unable to find symbol %s", s);
		return;
  }

	// look for symbol destroy_*
  ss.str(std::string());
	ss << "destroy_" << symbol;
	s = ss.str().c_str();
  destroy_storage_t* new_destroy_storage = (destroy_storage_t*)dlsym(dlhandle, ss.str().c_str());
  error = dlerror();
  if (error) {
    xlog("Unable to find symbol %s", s);
		return;
  }

	// register  valid symbols
	createStorage = new_create_storage;
	destroyStorage = new_destroy_storage;

	// start control
	if (createStorage) {
		instance = createStorage(params);
	}
}

Storage::~Storage() {
	delete instance;
}

void Storage::loco(const Loco& loco) {
}

vector<Loco*> Storage::allLocos() {
	return instance->allLocos();
}

} // namespace storage


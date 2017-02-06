#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include <vector>

#include "../datamodel/datamodel.h"

namespace storage {

class Storage {
	public:
		Storage(struct StorageParams& params);
		~Storage();
		void loco(const datamodel::Loco& loco);
		std::vector<datamodel::Loco*> allLocos();
	private:
		create_storage_t* createStorage;
		destroy_storage_t* destroyStorage;
		hardware::ControlInterface* instance;
		void* dlhandle;
		struct StorageParams params;

};

} // namespace storage

#endif // STORAGE_STORAGE_H

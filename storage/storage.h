#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include <vector>

#include "storage_interface.h"
#include "storage_params.h"
#include "../datamodel/datamodel.h"

namespace storage {

	// the types of the class factories
	typedef storage::StorageInterface* createStorage_t(struct StorageParams params);
	typedef void destroyStorage_t(storage::StorageInterface*);

	class Storage {
		public:
			Storage(struct StorageParams& params);
			~Storage();
			void loco(const datamodel::Loco& loco);
			std::vector<datamodel::Loco*> allLocos();
		private:
			createStorage_t* createStorage;
			destroyStorage_t* destroyStorage;
			storage::StorageInterface* instance;
			void* dlhandle;
			struct StorageParams params;

	};

} // namespace storage

#endif // STORAGE_STORAGE_H

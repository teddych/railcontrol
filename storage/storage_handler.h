#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include <map>

#include "datamodel/datamodel.h"
#include "datatypes.h"
#include "storage_interface.h"
#include "storage_params.h"

namespace storage {

	// the types of the class factories
	typedef storage::StorageInterface* createStorage_t(struct StorageParams params);
	typedef void destroyStorage_t(storage::StorageInterface*);

	class StorageHandler {
		public:
			StorageHandler(const StorageParams& params);
			~StorageHandler();
			void loco(const datamodel::Loco& loco);
			void allLocos(std::map<locoID_t,datamodel::Loco*>& locos);
		private:
			createStorage_t* createStorage;
			destroyStorage_t* destroyStorage;
			storage::StorageInterface* instance;
			void* dlhandle;
			const StorageParams params;
	};

} // namespace storage

#endif // STORAGE_STORAGE_H

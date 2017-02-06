#ifndef STORAGE_SQLITE_H
#define STORAGE_SQLITE_H

#include <vector>

#include "../datamodel/datamodel.h"
#include "storage_params.h"

namespace storage {

	class SQLite {
		public:
			SQLite(StorageParams& params);

			// save loco
			void loco(const datamodel::Loco& loco);

			// read all locos
			std::vector<datamodel::Loco*> allLocos();
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

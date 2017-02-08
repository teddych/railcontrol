#ifndef STORAGE_SQLITE_H
#define STORAGE_SQLITE_H

#include <vector>

#include "../datamodel/datamodel.h"
#include "storage_interface.h"
#include "storage_params.h"

namespace storage {

	class SQLite : public StorageInterface {
		public:
			SQLite(const StorageParams& params);
			~SQLite();

			// save loco
			void loco(const datamodel::Loco& loco);

			// read all locos
			std::vector<datamodel::Loco*> allLocos();

		private:
			const StorageParams params;
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

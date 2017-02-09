#ifndef STORAGE_SQLITE_H
#define STORAGE_SQLITE_H

#include <vector>

#include "datamodel/datamodel.h"
#include "storage_interface.h"
#include "storage_params.h"
#include "sqlite/sqlite3.h"

namespace storage {

	class SQLite : public StorageInterface {
		public:
			SQLite(const StorageParams& params);
			~SQLite();

			static int callbackListTables(void *v, int argc, char **argv, char **colName);

			// save loco
			void loco(const datamodel::Loco& loco);

			// read all locos
			std::vector<datamodel::Loco*> allLocos();

		private:
			const StorageParams& params;
			sqlite3 *db;
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

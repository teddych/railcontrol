#ifndef STORAGE_SQLITE_H
#define STORAGE_SQLITE_H

#include <map>

#include "datamodel/datamodel.h"
#include "storage_interface.h"
#include "storage_params.h"
#include "sqlite/sqlite3.h"

namespace storage {

	class SQLite : public StorageInterface {
		public:
			SQLite(const StorageParams& params);
			~SQLite();

			// save loco
			void loco(const datamodel::Loco& loco);

			// read all locos
			void allLocos(std::map<locoID_t, datamodel::Loco*>& locos);

		private:
			const StorageParams& params;
			sqlite3 *db;

			static int callbackListTables(void *v, int argc, char **argv, char **colName);
			static int callbackAllLocos(void* v, int argc, char **argv, char **colName);
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

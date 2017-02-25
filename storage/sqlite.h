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

			// save control
			void hardwareParams(const hardware::HardwareParams& params);

			// read controls
			void allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams);

			// save loco
			void loco(const datamodel::Loco& loco);

			// read all locos
			void allLocos(std::map<locoID_t, datamodel::Loco*>& locos);

			// save accessory
			void accessory(const datamodel::Accessory& accessory);

			// read all accessories
			void allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories);

		private:
			sqlite3 *db;

			static int callbackListTables(void *v, int argc, char **argv, char **colName);
			static int callbackAllHardwareParams(void *v, int argc, char **argv, char **colName);
			static int callbackAllLocos(void* v, int argc, char **argv, char **colName);
			static int callbackAllAccessories(void* v, int argc, char **argv, char **colName);
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

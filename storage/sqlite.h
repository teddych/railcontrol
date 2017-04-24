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
			void hardwareParams(const hardware::HardwareParams& params) override;

			// read controls
			void allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams) override;

			// save datamodelobject
			void saveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object) override;

			// read datamodelobject
			void objectsOfType(const objectType_t objectType, std::vector<std::string>& objects);

		private:
			sqlite3 *db;

			static int callbackListTables(void *v, int argc, char **argv, char **colName);
			static int callbackAllHardwareParams(void *v, int argc, char **argv, char **colName);
			static int callbackObjectsOfType(void* v, int argc, char **argv, char **colName);
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

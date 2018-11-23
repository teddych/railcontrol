#pragma once

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

			// delete control
			void deleteHardwareParams(const controlID_t controlID) override;

			// save datamodelobject
			void saveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object) override;

			// delete datamodelobject
			void deleteObject(const objectType_t objectType, const objectID_t objectID) override;

			// read datamodelobject
			void objectsOfType(const objectType_t objectType, std::vector<std::string>& objects) override;

			// save datamodelrelation
			void saveRelation(const relationType_t relationType, const objectID_t objectID1, const objectID_t objectID2, const std::string& name, const std::string& relation) override;

			// read datamodelrelation
			void relationsFromObject(const relationType_t relationType, const objectID_t objectID, std::vector<std::string>& relations) override;

			// read datamodelrelation
			void relationsToObject(const relationType_t relationType, const objectID_t objectID, std::vector<std::string>& relations) override;

		private:
			sqlite3 *db;
			const std::string filename;

			static int callbackListTables(void *v, int argc, char **argv, char **colName);
			static int callbackAllHardwareParams(void *v, int argc, char **argv, char **colName);
			static int callbackStringVector(void* v, int argc, char **argv, char **colName);
	};

} // namespace storage


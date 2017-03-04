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

			// save loco
			void loco(const datamodel::Loco& loco) override;

			// read all locos
			void allLocos(std::map<locoID_t, datamodel::Loco*>& locos) override;

			// save accessory
			void accessory(const datamodel::Accessory& accessory) override;

			// read all accessories
			void allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories) override;

			// save feedback
			void feedback(const datamodel::Feedback& feedback) override;

			// read all feedbacks
			void allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedback) override;

			// save block
			void block(const datamodel::Block& block) override;

			// read all blocks
			void allBlocks(std::map<blockID_t,datamodel::Block*>& block) override;

		private:
			sqlite3 *db;

			static int callbackListTables(void *v, int argc, char **argv, char **colName);
			static int callbackAllHardwareParams(void *v, int argc, char **argv, char **colName);
			static int callbackAllLocos(void* v, int argc, char **argv, char **colName);
			static int callbackAllAccessories(void* v, int argc, char **argv, char **colName);
			static int callbackAllFeedbacks(void* v, int argc, char **argv, char **colName);
			static int callbackAllBlocks(void* v, int argc, char **argv, char **colName);
	};

} // namespace storage

#endif // STORAGE_SQLITE_H

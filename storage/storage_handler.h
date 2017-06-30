#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include <map>

#include "datamodel/datamodel.h"
#include "datatypes.h"
#include "hardware/hardware_params.h"
#include "storage_interface.h"
#include "storage_params.h"

namespace storage {

	// the types of the class factories
	typedef storage::StorageInterface* createStorage_t(struct StorageParams params);
	typedef void destroyStorage_t(storage::StorageInterface*);

	class StorageHandler {
		public:
			StorageHandler(Manager* manager, const StorageParams& params);
			~StorageHandler();
			void hardwareParams(const hardware::HardwareParams& hardwareParams);
			void allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams);
			void loco(const datamodel::Loco& loco);
			void allLocos(std::map<locoID_t,datamodel::Loco*>& locos);
			void accessory(const datamodel::Accessory& accessory);
			void allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories);
			void feedback(const datamodel::Feedback& feedback);
			void allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks);
			void block(const datamodel::Block& block);
			void allBlocks(std::map<blockID_t,datamodel::Block*>& blocks);
			void saveSwitch(const datamodel::Switch& mySwitch);
			void allSwitches(std::map<switchID_t,datamodel::Switch*>& switches);
			void street(const datamodel::Street& street);
			void allStreets(std::map<streetID_t,datamodel::Street*>& streets);
		private:
			Manager* manager;
			createStorage_t* createStorage;
			destroyStorage_t* destroyStorage;
			storage::StorageInterface* instance;
			void* dlhandle;
	};

} // namespace storage

#endif // STORAGE_STORAGE_H

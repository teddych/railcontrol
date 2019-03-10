#pragma once

#include <map>

#include "datamodel/datamodel.h"
#include "datatypes.h"
#include "hardware/HardwareParams.h"
#include "storage/StorageInterface.h"
#include "storage/StorageParams.h"

namespace storage
{

	// the types of the class factories
	typedef storage::StorageInterface* createStorage_t(struct StorageParams params);
	typedef void destroyStorage_t(storage::StorageInterface*);

	class StorageHandler
	{
		public:
			StorageHandler(Manager* manager, const StorageParams& params);
			~StorageHandler();
			void allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams);
			void deleteHardwareParams(const controlID_t controlID);
			void allLocos(std::map<locoID_t,datamodel::Loco*>& locos);
			void deleteLoco(locoID_t locoID);
			void allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories);
			void deleteAccessory(accessoryID_t accessoryID);
			void allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks);
			void deleteFeedback(feedbackID_t feedbackID);
			void allTracks(std::map<trackID_t,datamodel::Track*>& tracks);
			void deleteTrack(trackID_t trackID);
			void allSwitches(std::map<switchID_t,datamodel::Switch*>& switches);
			void deleteSwitch(switchID_t switchID);
			void allStreets(std::map<streetID_t,datamodel::Street*>& streets);
			void deleteStreet(streetID_t streetID);
			void allLayers(std::map<layerID_t,datamodel::Layer*>& layers);
			void deleteLayer(layerID_t layerID);
			void Save(const hardware::HardwareParams& hardwareParams);
			void Save(const datamodel::Loco& loco);
			void Save(const datamodel::Accessory& accessory);
			void Save(const datamodel::Feedback& feedback);
			void Save(const datamodel::Track& track);
			void Save(const datamodel::Switch& mySwitch);
			void Save(const datamodel::Street& street);
			void Save(const datamodel::Layer& layer);
			template <class T> static void Save(StorageHandler* storageHandler, const T* t) { storageHandler->Save(*t); }

		private:
			Manager* manager;
			createStorage_t* createStorage;
			destroyStorage_t* destroyStorage;
			storage::StorageInterface* instance;
			void* dlhandle;
	};

} // namespace storage


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
			void AllHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams);
			void DeleteHardwareParams(const controlID_t controlID);
			void AllLocos(std::map<locoID_t,datamodel::Loco*>& locos);
			void DeleteLoco(locoID_t locoID);
			void AllAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories);
			void DeleteAccessory(accessoryID_t accessoryID);
			void AllFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks);
			void DeleteFeedback(feedbackID_t feedbackID);
			void AllTracks(std::map<trackID_t,datamodel::Track*>& tracks);
			void DeleteTrack(trackID_t trackID);
			void AllSwitches(std::map<switchID_t,datamodel::Switch*>& switches);
			void DeleteSwitch(switchID_t switchID);
			void AllStreets(std::map<streetID_t,datamodel::Street*>& streets);
			void DeleteStreet(streetID_t streetID);
			void AllLayers(std::map<layerID_t,datamodel::Layer*>& layers);
			void DeleteLayer(layerID_t layerID);
			void Save(const hardware::HardwareParams& hardwareParams);
			void Save(const datamodel::Loco& loco);
			void Save(const datamodel::Accessory& accessory);
			void Save(const datamodel::Feedback& feedback);
			void Save(const datamodel::Track& track);
			void Save(const datamodel::Switch& mySwitch);
			void Save(const datamodel::Street& street);
			void Save(const datamodel::Layer& layer);
			template <class T> static void Save(StorageHandler* storageHandler, const T* t) { storageHandler->Save(*t); }
			void SaveSetting(const std::string& key, const std::string& value);
			std::string GetSetting(const std::string& key) ;
			void StartTransaction();
			void CommitTransaction();

		private:
			void StartTransactionInternal();
			void CommitTransactionInternal();

			Manager* manager;
			createStorage_t* createStorage;
			destroyStorage_t* destroyStorage;
			storage::StorageInterface* instance;
			void* dlhandle;
			bool transactionRunning;
	};

} // namespace storage


#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "config.h"
#include "ControlInterface.h"
#include "datamodel/datamodel.h"
#include "hardware/HardwareParams.h"
#include "Logger/Logger.h"
#include "storage/StorageHandler.h"

class DelayedCall;

class Manager {
	public:
		Manager(Config& config);
		~Manager();

		// booster
		boosterState_t Booster() const { return boosterState; }
		void Booster(const controlType_t controlType, const boosterState_t status);

		// hardware (virt, CS2, ...)
		static const std::map<hardwareType_t,std::string> hardwareListNames();
		bool controlSave(const controlID_t& controlID,
			const hardwareType_t& hardwareType,
			const std::string& name,
			const std::string& arg1,
			const std::string& arg2,
			const std::string& arg3,
			const std::string& arg4,
			const std::string& arg5,
			std::string& result);
		bool controlDelete(controlID_t controlID);
		hardware::HardwareParams* getHardware(controlID_t controlID);
		unsigned int controlsOfHardwareType(const hardwareType_t hardwareType);
		bool hardwareLibraryAdd(const hardwareType_t hardwareType, void* libraryHandle);
		void* hardwareLibraryGet(const hardwareType_t hardwareType) const;
		bool hardwareLibraryRemove(const hardwareType_t hardwareType);

		// control (console, web, ...)
		const std::string getControlName(const controlID_t controlID); // FIXME: => string& (reference)
		const std::map<controlID_t,hardware::HardwareParams*> controlList() const { return hardwareParams; }
		const std::map<std::string,hardware::HardwareParams*> controlListByName() const;
		const std::map<controlID_t,std::string> LocoControlListNames() const;
		const std::map<controlID_t,std::string> AccessoryControlListNames() const;
		const std::map<controlID_t,std::string> FeedbackControlListNames() const;
		const std::map<std::string,protocol_t> LocoProtocolsOfControl(const controlID_t controlID) const { return ProtocolsOfControl(AddressTypeLoco, controlID); }
		const std::map<std::string,protocol_t> AccessoryProtocolsOfControl(const controlID_t controlID) const { return ProtocolsOfControl(AddressTypeAccessory, controlID); }
		const std::map<unsigned char,argumentType_t> ArgumentTypesOfControl(const controlID_t controlID) const;

		// loco
		datamodel::Loco* GetLoco(const locoID_t locoID) const;
		const std::string& LocoName(const locoID_t locoID) const;
		const std::map<locoID_t,datamodel::Loco*>& locoList() const { return locos; }
		const std::map<std::string,locoID_t> LocoListFree() const;
		const std::map<std::string,datamodel::Loco*> LocoListByName() const;
		bool locoSave(const locoID_t locoID, const std::string& name, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t nr, std::string& result);
		bool locoDelete(const locoID_t locoID);
		bool locoProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		void LocoSpeed(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const locoSpeed_t speed);
		bool LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed);
		const locoSpeed_t LocoSpeed(const locoID_t locoID) const;
		void LocoDirection(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const direction_t direction);
		void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction);
		void LocoFunction(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t function, const bool on);
		void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on);

		// accessory
		void AccessoryState(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryState_t state);
		void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state);
		void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const bool on);
		datamodel::Accessory* getAccessory(const accessoryID_t accessoryID) const;
		const std::string& getAccessoryName(const accessoryID_t accessoryID) const;
		const std::map<accessoryID_t,datamodel::Accessory*>& accessoryList() const { return accessories; }
		const std::map<std::string,datamodel::Accessory*> accessoryListByName() const;
		bool accessorySave(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryTimeout_t timeout, const bool inverted, std::string& result);
		bool accessoryDelete(const accessoryID_t accessoryID);
		bool accessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// feedback
		void FeedbackState(const controlType_t controlType, const controlID_t controlID, const feedbackPin_t pin, const feedbackState_t state);
		void FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state);
		datamodel::Feedback* GetFeedback(feedbackID_t feedbackID) const;
		const std::string& GetFeedbackName(const feedbackID_t feedbackID) const;
		const std::map<feedbackID_t,datamodel::Feedback*>& feedbackList() const { return feedbacks; }
		const std::map<std::string,datamodel::Feedback*> FeedbackListByName() const;
		const std::map<std::string,feedbackID_t> FeedbacksOfTrack(const trackID_t trackID) const;
		feedbackID_t FeedbackSave(const feedbackID_t feedbackID, const std::string& name, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, const bool inverted,  std::string& result);
		bool FeedbackDelete(const feedbackID_t feedbackID);

		// track
		void track(const controlType_t controlType, const feedbackID_t feedbackID, const lockState_t);
		datamodel::Track* GetTrack(const trackID_t trackID) const;
		const std::string& GetTrackName(const trackID_t trackID) const;
		const std::map<trackID_t,datamodel::Track*>& trackList() const { return tracks; }
		const std::map<std::string,datamodel::Track*> trackListByName() const;
		const std::map<std::string,trackID_t> trackListIdByName() const;
		trackID_t TrackSave(const trackID_t trackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutRotation_t rotation, const trackType_t trackType, std::vector<feedbackID_t> feedbacks, std::string& result);
		bool TrackDelete(const trackID_t trackID);
		bool TrackSetFeedbackState(const trackID_t trackID, const feedbackID_t feedbackID, const feedbackState_t state, const std::string& locoName);

		// switch
		void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state);
		void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const bool on);
		datamodel::Switch* getSwitch(const switchID_t switchID) const;
		const std::string& getSwitchName(const switchID_t switchID) const;
		const std::map<switchID_t,datamodel::Switch*>& switchList() const { return switches; }
		const std::map<std::string,datamodel::Switch*> switchListByName() const;
		bool switchSave(const switchID_t switchID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const switchType_t type, const switchTimeout_t timeout, const bool inverted, std::string& result);
		bool switchDelete(const switchID_t switchID);
		bool switchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// street
		void ExecuteStreet(const streetID_t streetID);
		void ExecuteStreetAsync(const streetID_t streetID);
		datamodel::Street* GetStreet(const streetID_t streetID) const;
		const std::string& getStreetName(const streetID_t streetID) const;
		const std::map<streetID_t,datamodel::Street*>& streetList() const { return streets; }
		const std::map<std::string,datamodel::Street*> streetListByName() const;
		bool StreetSave(const streetID_t streetID, const std::string& name, const delay_t delay, const std::vector<datamodel::Relation*>& relations, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const automode_t automode, const trackID_t fromTrack, const direction_t fromDirection, const trackID_t toTrack, const direction_t toDirection, const feedbackID_t feedbackID, std::string& result);
		bool streetDelete(const streetID_t streetID);

		// layer
		datamodel::Layer* GetLayer(const layerID_t layerID) const;
		const std::map<std::string,layerID_t> LayerListByName() const;
		const std::map<std::string,layerID_t> LayerListByNameWithFeedback() const;
		bool LayerSave(const layerID_t layerID, const std::string&name, std::string& result);
		bool LayerDelete(const layerID_t layerID);

		// automode
		bool LocoIntoTrack(const locoID_t locoID, const trackID_t trackID);
		bool LocoRelease(const locoID_t locoID);
		bool TrackRelease(const trackID_t trackID);
		bool TrackStartLoco(const trackID_t trackID);
		bool TrackStopLoco(const trackID_t trackID);
		bool feedbackRelease(const feedbackID_t feedbackID);
		bool StreetRelease(const streetID_t streetID);
		//bool switchRelease(const switchID_t switchID);
		bool LocoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID, const std::string& locoName);
		bool LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID);
		bool LocoStart(const locoID_t locoID);
		bool LocoStop(const locoID_t locoID);
		bool locoStartAll();
		bool locoStopAll();
		void StopAllLocosImmediately(const controlType_t controlType);

	private:
		// layout
		bool CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, std::string& result) const;
		bool CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, std::string& result) const;
		template<class Type> bool CheckLayoutPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, std::string& result, const std::map<objectID_t, Type*>& layoutVector, std::mutex& mutex) const;
		bool CheckAccessoryPosition(const accessoryID_t accessoryID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;
		bool CheckSwitchPosition(const switchID_t switchID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;
		bool CheckStreetPosition(const streetID_t streetID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;
		bool CheckTrackPosition(const trackID_t trackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const layoutRotation_t rotation, std::string& result) const;
		bool CheckFeedbackPosition(const feedbackID_t feedbackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;

		bool checkAddressLoco(const protocol_t protocol, const address_t address, std::string& result);
		bool checkAddressAccessory(const protocol_t protocol, const address_t address, std::string& result);
		bool checkControlLocoProtocolAddress(const controlID_t controlID, const protocol_t protocol, const address_t address, std::string& result)
		{
			return checkControlProtocolAddress(AddressTypeLoco, controlID, protocol, address, result);
		}
		bool checkControlAccessoryProtocolAddress(const controlID_t controlID, const protocol_t protocol, const address_t address, std::string& result)
		{
			return checkControlProtocolAddress(AddressTypeAccessory, controlID, protocol, address, result);
		}
		bool checkControlProtocolAddress(const addressType_t type, const controlID_t controlID, const protocol_t protocol, const address_t address, std::string& result);
		const std::map<std::string,protocol_t> ProtocolsOfControl(const addressType_t type, const controlID_t) const;

		bool LocoReleaseInternal(const locoID_t locoID);
		bool TrackReleaseInternal(const trackID_t trackID);
		bool TrackReleaseInternal(datamodel::Track* track);

		template<class Key, class Value>
		void DeleteAllMapEntries(std::map<Key,Value*>& m, std::mutex& x, storage::StorageHandler* storage = nullptr)
		{
			std::lock_guard<std::mutex> Guard(x);
			while (m.size())
			{
				auto it = m.begin();
				Value* content = it->second;
				m.erase(it);
				if (storage != nullptr)
				{
					storage->Save(*content);
				}
				logger->Info("Saving {0}", content->Name());
				delete content;
			}
		}

		const std::vector<feedbackID_t> CleanupAndCheckFeedbacks(trackID_t trackID, std::vector<feedbackID_t>& newFeedbacks);

		Logger::Logger* logger;
		boosterState_t boosterState;

		// FIXME: check usage of all mutexes
		// const hardwareType_t hardwareOfControl(controlID_t controlID) const;

		// controls (Webserver, console & hardwareHandler. So each hardware is also added here).
		std::map<controlID_t,ControlInterface*> controls;
		mutable std::mutex controlMutex;

		// hardware (virt, CS2, ...)
		std::map<controlID_t,hardware::HardwareParams*> hardwareParams;
		mutable std::mutex hardwareMutex;

		std::map<hardwareType_t,void*> hardwareLibraries;
		mutable std::mutex hardwareLibrariesMutex;

		// loco
		std::map<locoID_t,datamodel::Loco*> locos;
		mutable std::mutex locoMutex;

		// accessory
		std::map<accessoryID_t,datamodel::Accessory*> accessories;
		mutable std::mutex accessoryMutex;

		// feedback
		std::map<feedbackID_t,datamodel::Feedback*> feedbacks;
		mutable std::mutex feedbackMutex;

		// track
		std::map<trackID_t,datamodel::Track*> tracks;
		mutable std::mutex trackMutex;

		// switch
		std::map<switchID_t,datamodel::Switch*> switches;
		mutable std::mutex switchMutex;

		// street
		std::map<streetID_t,datamodel::Street*> streets;
		mutable std::mutex streetMutex;

		// layer
		std::map<layerID_t,datamodel::Layer*> layers;
		mutable std::mutex layerMutex;

		// storage
		storage::StorageHandler* storage;
		DelayedCall* delayedCall;

		const std::string unknownControl;
		const std::string unknownLoco;
		const std::string unknownAccessory;
		const std::string unknownFeedback;
		const std::string unknownTrack;
		const std::string unknownSwitch;
		const std::string unknownStreet;
};

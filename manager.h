#pragma once

#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <iomanip>
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
		static const std::map<hardwareType_t,std::string> HardwareListNames();
		bool ControlSave(const controlID_t& controlID,
			const hardwareType_t& hardwareType,
			const std::string& name,
			const std::string& arg1,
			const std::string& arg2,
			const std::string& arg3,
			const std::string& arg4,
			const std::string& arg5,
			std::string& result);
		bool ControlDelete(controlID_t controlID);
		hardware::HardwareParams* GetHardware(controlID_t controlID);
		unsigned int ControlsOfHardwareType(const hardwareType_t hardwareType);
		bool HardwareLibraryAdd(const hardwareType_t hardwareType, void* libraryHandle);
		void* HardwareLibraryGet(const hardwareType_t hardwareType) const;
		bool HardwareLibraryRemove(const hardwareType_t hardwareType);

		// control (console, web, ...)
		const std::string GetControlName(const controlID_t controlID); // FIXME: => string& (reference)
		const std::map<controlID_t,hardware::HardwareParams*> controlList() const { return hardwareParams; }
		const std::map<std::string,hardware::HardwareParams*> ControlListByName() const;
		const std::map<controlID_t,std::string> LocoControlListNames() const;
		const std::map<controlID_t,std::string> AccessoryControlListNames() const;
		const std::map<controlID_t,std::string> FeedbackControlListNames() const;
		const std::map<std::string,protocol_t> LocoProtocolsOfControl(const controlID_t controlID) const { return ProtocolsOfControl(AddressTypeLoco, controlID); }
		const std::map<std::string,protocol_t> AccessoryProtocolsOfControl(const controlID_t controlID) const { return ProtocolsOfControl(AddressTypeAccessory, controlID); }
		const std::map<unsigned char,argumentType_t> ArgumentTypesOfControl(const controlID_t controlID) const;

		// loco
		datamodel::Loco* GetLoco(const locoID_t locoID) const;
		const std::string& GetLocoName(const locoID_t locoID) const;
		const std::map<locoID_t,datamodel::Loco*>& locoList() const { return locos; }
		const std::map<std::string,locoID_t> LocoListFree() const;
		const std::map<std::string,datamodel::Loco*> LocoListByName() const;
		bool LocoSave
		(
			const locoID_t locoID,
			const std::string& name,
			const controlID_t controlID,
			const protocol_t protocol,
			const address_t address,
			const function_t nrOfFunctions,
			const length_t length,
			const bool commuter,
			const locoSpeed_t maxSpeed,
			const locoSpeed_t travelSpeed,
			const locoSpeed_t reducedSpeed,
			const locoSpeed_t creepSpeed,
			std::string& result
		);
		bool LocoDelete(const locoID_t locoID);
		bool LocoProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		void LocoSpeed(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const locoSpeed_t speed);
		bool LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed);
		bool LocoSpeed(const controlType_t controlType, datamodel::Loco* loco, const locoSpeed_t speed);
		const locoSpeed_t LocoSpeed(const locoID_t locoID) const;
		void LocoDirection(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const direction_t direction);
		void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction);
		void LocoDirection(const controlType_t controlType, datamodel::Loco* loco, const direction_t direction);
		void LocoFunction(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t function, const bool on);
		void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on);

		// accessory
		void AccessoryState(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryState_t state);
		void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool force);
		void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const bool on);
		datamodel::Accessory* GetAccessory(const accessoryID_t accessoryID) const;
		const std::string& GetAccessoryName(const accessoryID_t accessoryID) const;
		const std::map<accessoryID_t,datamodel::Accessory*>& AccessoryList() const { return accessories; }
		const std::map<std::string,datamodel::Accessory*> AccessoryListByName() const;
		bool AccessorySave(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryDuration_t timeout, const bool inverted, std::string& result);
		bool AccessoryDelete(const accessoryID_t accessoryID);
		bool AccessoryRelease(const accessoryID_t accessoryID);
		bool AccessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// feedback
		void FeedbackState(const controlID_t controlID, const feedbackPin_t pin, const feedbackState_t state);
		void FeedbackState(const feedbackID_t feedbackID, const feedbackState_t state);
		void FeedbackState(datamodel::Feedback* feedback);
		datamodel::Feedback* GetFeedback(const feedbackID_t feedbackID) const;
		datamodel::Feedback* GetFeedbackUnlocked(const feedbackID_t feedbackID) const;
		const std::string& GetFeedbackName(const feedbackID_t feedbackID) const;
		const std::map<feedbackID_t,datamodel::Feedback*>& FeedbackList() const { return feedbacks; }
		const std::map<std::string,datamodel::Feedback*> FeedbackListByName() const;
		const std::map<std::string,feedbackID_t> FeedbacksOfTrack(const trackID_t trackID) const;
		feedbackID_t FeedbackSave(const feedbackID_t feedbackID, const std::string& name, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, const bool inverted,  std::string& result);
		bool FeedbackDelete(const feedbackID_t feedbackID);

		// track
		datamodel::Track* GetTrack(const trackID_t trackID) const;
		const std::string& GetTrackName(const trackID_t trackID) const;
		const std::map<trackID_t,datamodel::Track*>& TrackList() const { return tracks; }
		const std::map<std::string,datamodel::Track*> TrackListByName() const;
		const std::map<std::string,trackID_t> TrackListIdByName() const;
		trackID_t TrackSave(const trackID_t trackID,
			const std::string& name,
			const layoutPosition_t posX,
			const layoutPosition_t posY,
			const layoutPosition_t posZ,
			const layoutItemSize_t width,
			const layoutRotation_t rotation,
			const trackType_t trackType,
			std::vector<feedbackID_t> feedbacks,
			const datamodel::Track::selectStreetApproach_t selectStreetApproach,
			std::string& result);
		bool TrackDelete(const trackID_t trackID);

		// switch
		void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool force);
		void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const bool on);
		datamodel::Switch* GetSwitch(const switchID_t switchID) const;
		const std::string& GetSwitchName(const switchID_t switchID) const;
		const std::map<switchID_t,datamodel::Switch*>& SwitchList() const { return switches; }
		const std::map<std::string,datamodel::Switch*> SwitchListByName() const;
		bool SwitchSave(const switchID_t switchID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const switchType_t type, const switchDuration_t timeout, const bool inverted, std::string& result);
		bool SwitchDelete(const switchID_t switchID);
		bool SwitchRelease(const switchID_t switchID);
		bool SwitchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// street
		void ExecuteStreet(const streetID_t streetID);
		void ExecuteStreetAsync(const streetID_t streetID);
		datamodel::Street* GetStreet(const streetID_t streetID) const;
		const std::string& GetStreetName(const streetID_t streetID) const;
		const std::map<streetID_t,datamodel::Street*>& StreetList() const { return streets; }
		const std::map<std::string,datamodel::Street*> StreetListByName() const;
		bool StreetSave(const streetID_t streetID,
			const std::string& name,
			const delay_t delay,
			const datamodel::Street::commuterType_t commuter,
			const length_t minTrainLength,
			const length_t maxTrainLength,
			const std::vector<datamodel::Relation*>& relations,
			const visible_t visible,
			const layoutPosition_t posX,
			const layoutPosition_t posY,
			const layoutPosition_t posZ,
			const automode_t automode,
			const trackID_t fromTrack,
			const direction_t fromDirection,
			const trackID_t toTrack,
			const direction_t toDirection,
			const feedbackID_t feedbackIdReduced,
			const feedbackID_t feedbackIdCreep,
			const feedbackID_t feedbackIdStop,
			const feedbackID_t feedbackIdOver,
			std::string& result);
		bool StreetDelete(const streetID_t streetID);

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
		bool TrackReleaseWithLoco(const trackID_t trackID);
		bool TrackStartLoco(const trackID_t trackID);
		bool TrackStopLoco(const trackID_t trackID);
		void TrackBlock(const trackID_t trackID, const bool blocked);
		void TrackPublishState(const datamodel::Track* track);
		bool StreetRelease(const streetID_t streetID);
		bool LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID);
		bool LocoStart(const locoID_t locoID);
		bool LocoStop(const locoID_t locoID);
		bool LocoStartAll();
		bool LocoStopAll();
		void StopAllLocosImmediately(const controlType_t controlType);

		// settings
		accessoryDuration_t GetDefaultAccessoryDuration() const { return defaultAccessoryDuration; }
		bool GetAutoAddFeedback() const { return autoAddFeedback; }
		datamodel::Track::selectStreetApproach_t GetSelectStreetApproach() const { return selectStreetApproach; }
		bool SaveSettings
		(
			const accessoryDuration_t duration,
			const bool autoAddFeedback,
			const datamodel::Track::selectStreetApproach_t selectStreetApproach
		);

	private:
		const ControlInterface* GetControl(const controlID_t controlID) const;
		datamodel::Loco* GetLoco(const controlID_t controlID, const protocol_t protocol, const address_t address) const;
		datamodel::Accessory* GetAccessory(const controlID_t controlID, const protocol_t protocol, const address_t address) const;
		datamodel::Switch* GetSwitch(const controlID_t controlID, const protocol_t protocol, const address_t address) const;
		datamodel::Feedback* GetFeedback(const controlID_t controlID, const feedbackPin_t pin) const;

		void LocoFunction(const controlType_t controlType, datamodel::Loco* loco, const function_t function, const bool on);
		void AccessoryState(const controlType_t controlType, datamodel::Accessory* accessory, const accessoryState_t state, const bool force);
		void SwitchState(const controlType_t controlType, datamodel::Switch* mySwitch, const accessoryState_t state, const bool force);
		void FeedbackState(datamodel::Feedback* feedback, const feedbackState_t state);

		// layout
		bool CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, std::string& result) const;
		bool CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, std::string& result) const;
		template<class Type> bool CheckLayoutPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, std::string& result, const std::map<objectID_t, Type*>& layoutVector, std::mutex& mutex) const;
		bool CheckAccessoryPosition(const accessoryID_t accessoryID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;
		bool CheckSwitchPosition(const switchID_t switchID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;
		bool CheckStreetPosition(const streetID_t streetID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;
		bool CheckTrackPosition(const trackID_t trackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const layoutRotation_t rotation, std::string& result) const;
		bool CheckFeedbackPosition(const feedbackID_t feedbackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const;

		bool CheckAddressLoco(const protocol_t protocol, const address_t address, std::string& result);
		bool CheckAddressAccessory(const protocol_t protocol, const address_t address, std::string& result);
		bool CheckControlLocoProtocolAddress(const controlID_t controlID, const protocol_t protocol, const address_t address, std::string& result)
		{
			return CheckControlProtocolAddress(AddressTypeLoco, controlID, protocol, address, result);
		}
		bool CheckControlAccessoryProtocolAddress(const controlID_t controlID, const protocol_t protocol, const address_t address, std::string& result)
		{
			return CheckControlProtocolAddress(AddressTypeAccessory, controlID, protocol, address, result);
		}
		bool CheckControlProtocolAddress(const addressType_t type, const controlID_t controlID, const protocol_t protocol, const address_t address, std::string& result);
		const std::map<std::string,protocol_t> ProtocolsOfControl(const addressType_t type, const controlID_t) const;

		bool LocoReleaseInternal(const locoID_t locoID);
		bool TrackReleaseInternal(const trackID_t trackID);
		bool TrackReleaseInternal(datamodel::Track* track);

		template<class Key, class Value>
		void DeleteAllMapEntries(std::map<Key,Value*>& m, std::mutex& x)
		{
			std::lock_guard<std::mutex> Guard(x);
			while (m.size())
			{
				auto it = m.begin();
				Value* content = it->second;
				m.erase(it);
				if (storage != nullptr)
				{
					logger->Info("Saving {0}", content->GetName());
					storage->Save(*content);
				}
				delete content;
			}
		}

		const std::vector<feedbackID_t> CleanupAndCheckFeedbacks(trackID_t trackID, std::vector<feedbackID_t>& newFeedbacks);
		void DebounceWorker(Manager* manager);

		template<class ID, class T>
		T* CreateAndAddObject(std::map<ID,T*>& objects, std::mutex& mutex)
		{
			std::lock_guard<std::mutex> Guard(mutex);
			ID newObjectID = 0;
			for (auto object : objects)
			{
				if (object.first > newObjectID)
				{
					newObjectID = object.first;
				}
			}
			++newObjectID;
			T* newObject = new T(this, newObjectID);
			if (newObject == nullptr)
			{
				return nullptr;
			}
			objects[newObjectID] = newObject;
			return newObject;
		}

		Logger::Logger* logger;
		boosterState_t boosterState;

		template<class ID, class T>
		bool CheckObjectName(std::map<ID,T*>& objects, const std::string& name)
		{
			for (auto object : objects)
			{
				if (object.second->GetName().compare(name) == 0)
				{
					return false;
				}
			}
			return true;
		}

		bool CheckIfNumber(const char& c) { return c >= '0' && c <= '9'; }
		bool CheckIfThreeNumbers(const std::string& s)
		{
			size_t sSize = s.size();
			return sSize >= 3
			&& CheckIfNumber(s.at(sSize-1))
			&& CheckIfNumber(s.at(sSize-2))
			&& CheckIfNumber(s.at(sSize-3));
		}

		template<class ID, class T>
		std::string CheckObjectName(std::map<ID,T*>& objects, std::mutex& mutex, const std::string& name)
		{
			std::lock_guard<std::mutex> Guard(mutex);
			if (CheckObjectName(objects, name))
			{
				return name;
			}
			unsigned int counter = 0;

			const std::string baseName = CheckIfThreeNumbers(name) ? name.substr(0, name.size() - 3) : name;

			while (true)
			{
				++counter;
				std::stringstream ss;
				ss << baseName << std::setw(3) << std::setfill('0') << counter;
				std::string newName = ss.str();
				if (CheckObjectName(objects, newName))
				{
					return newName;
				}
			}
		}

		// FIXME: check usage of all mutexes

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

		accessoryDuration_t defaultAccessoryDuration;
		bool autoAddFeedback;
		datamodel::Track::selectStreetApproach_t selectStreetApproach;

		volatile bool debounceRun;
		std::thread debounceThread;

		const std::string unknownControl;
		const std::string unknownLoco;
		const std::string unknownAccessory;
		const std::string unknownFeedback;
		const std::string unknownTrack;
		const std::string unknownSwitch;
		const std::string unknownStreet;
};

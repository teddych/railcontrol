#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "command_interface.h"
#include "config.h"
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
		void booster(const controlType_t controlType, const boosterStatus_t status);

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
		inline const std::map<controlID_t,hardware::HardwareParams*> controlList() const { return hardwareParams; }
		const std::map<std::string,hardware::HardwareParams*> controlListByName() const;
		const std::map<controlID_t,std::string> LocoControlListNames() const;
		const std::map<controlID_t,std::string> AccessoryControlListNames() const;
		const std::map<std::string,protocol_t> LocoProtocolsOfControl(const controlID_t controlID) const { return ProtocolsOfControl(AddressTypeLoco, controlID); }
		const std::map<std::string,protocol_t> AccessoryProtocolsOfControl(const controlID_t controlID) const { return ProtocolsOfControl(AddressTypeAccessory, controlID); }
		const std::map<unsigned char,argumentType_t> ArgumentTypesOfControl(const controlID_t controlID) const;

		// loco
		datamodel::Loco* getLoco(const locoID_t locoID) const;
		const std::string& getLocoName(const locoID_t locoID);
		inline const std::map<locoID_t,datamodel::Loco*>& locoList() const { return locos; }
		const std::map<std::string,datamodel::Loco*> locoListByName() const;
		bool locoSave(const locoID_t locoID, const std::string& name, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t nr, std::string& result);
		bool locoDelete(const locoID_t locoID);
		bool locoProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		void locoSpeed(const controlType_t controlType, const protocol_t protocol, const address_t address, const LocoSpeed speed);
		bool locoSpeed(const controlType_t controlType, const locoID_t locoID, const LocoSpeed speed);
		const LocoSpeed locoSpeed(const locoID_t locoID) const;
		void locoDirection(const controlType_t controlType, const protocol_t protocol, const address_t address, const direction_t direction);
		void locoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction);
		void locoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on);

		// accessory
		void accessory(const controlType_t controlType, const protocol_t protocol, const address_t address, const accessoryState_t state);
		void accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state);
		void accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const bool on);
		datamodel::Accessory* getAccessory(const accessoryID_t accessoryID);
		const std::string& getAccessoryName(const accessoryID_t accessoryID);
		inline const std::map<accessoryID_t,datamodel::Accessory*>& accessoryList() const { return accessories; }
		bool accessorySave(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryTimeout_t timeout, const bool inverted, std::string& result);
		bool accessoryDelete(const accessoryID_t accessoryID);
		bool accessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// feedback
		void feedback(const controlType_t controlType, const feedbackPin_t pin, const feedbackState_t state);
		datamodel::Feedback* getFeedback(feedbackID_t feedbackID);
		const std::string& getFeedbackName(const feedbackID_t feedbackID);
		inline const std::map<feedbackID_t,datamodel::Feedback*>& feedbackList() const { return feedbacks; }
		bool feedbackSave(const feedbackID_t feedbackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, const bool inverted,  std::string& result);
		bool feedbackDelete(const feedbackID_t feedbackID);

		// track
		void track(const controlType_t controlType, const feedbackID_t feedbackID, const lockState_t);
		datamodel::Track* getTrack(const trackID_t trackID);
		const std::string& getTrackName(const trackID_t trackID);
		inline const std::map<trackID_t,datamodel::Track*>& trackList() const { return tracks; }
		const std::map<std::string,datamodel::Track*> trackListByName() const;
		bool trackSave(const trackID_t trackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutRotation_t rotation, const trackType_t trackType, std::string& result);
		bool trackDelete(const trackID_t trackID);

		// switch
		void handleSwitch(const controlType_t controlType, const switchID_t switchID, const switchState_t state);
		void handleSwitch(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const bool on);
		datamodel::Switch* getSwitch(const switchID_t switchID);
		const std::string& getSwitchName(const switchID_t switchID);
		inline const std::map<switchID_t,datamodel::Switch*>& switchList() const { return switches; }
		const std::map<std::string,datamodel::Switch*> switchListByName() const;
		bool switchSave(const switchID_t switchID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const switchType_t type, const switchTimeout_t timeout, const bool inverted, std::string& result);
		bool switchDelete(const switchID_t switchID);
		bool switchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// street
		datamodel::Street* getStreet(const streetID_t streetID);
		const std::string& getStreetName(const streetID_t streetID);
		inline const std::map<streetID_t,datamodel::Street*>& streetList() const { return streets; }
		bool streetSave(const streetID_t streetID, const std::string& name, const trackID_t fromTrack, const direction_t fromDirection, const trackID_t toTrack, const direction_t toDirection, const feedbackID_t feedbackID, std::string& result);
		bool streetDelete(const streetID_t streetID);

		// automode
		bool locoIntoTrack(const locoID_t locoID, const trackID_t trackID);
		bool locoRelease(const locoID_t locoID);
		bool trackRelease(const trackID_t trackID);
		bool feedbackRelease(const feedbackID_t feedbackID);
		bool streetRelease(const streetID_t streetID);
		//bool switchRelease(const switchID_t switchID);
		bool locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID);
		bool locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID);
		bool locoStart(const locoID_t locoID);
		bool locoStop(const locoID_t locoID);
		bool locoStartAll();
		bool locoStopAll();
		void StopAllLocosImmediately(const controlType_t controlType);

	private:
		void loadDefaultValuesToDB();

		// layout
		bool checkPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, std::string& result);
		bool checkPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, std::string& result);
		template<class Type> bool checkLayoutPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, std::string& result, std::map<objectID_t, Type*>& layoutVector, std::mutex& mutex);
		bool checkAccessoryPosition(const accessoryID_t accessoryID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ);
		bool checkSwitchPosition(const switchID_t switchID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ);
		bool checkTrackPosition(const trackID_t trackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const layoutRotation_t rotation, std::string& result);

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

		Logger::Logger* logger;

		// FIXME: check usage of all mutexes
		// const hardwareType_t hardwareOfControl(controlID_t controlID) const;

		// controls (Webserver, console & hardwareHandler. So each hardware is also added here).
		std::map<controlID_t,CommandInterface*> controls;
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
		std::mutex accessoryMutex;

		// feedback
		std::map<feedbackID_t,datamodel::Feedback*> feedbacks;
		std::mutex feedbackMutex;

		// track
		std::map<trackID_t,datamodel::Track*> tracks;
		mutable std::mutex trackMutex;

		// switch
		std::map<switchID_t,datamodel::Switch*> switches;
		mutable std::mutex switchMutex;

		// street
		std::map<streetID_t,datamodel::Street*> streets;
		std::mutex streetMutex;

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

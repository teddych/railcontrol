#ifndef MANAGER_H
#define MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "config.h"
#include "datamodel/datamodel.h"
#include "manager_interface.h"
#include "storage/storage_handler.h"
#include "hardware/hardware_params.h"

class Manager {
	public:
		Manager(Config& config);
		~Manager();
		void loadDefaultValuesToDB();

		// booster
		void booster(const managerID_t managerID, const boosterStatus_t status);

		// hardware (virt, CS2, ...)
		void saveHardware(const controlID_t& controlID, const hardwareID_t& hardwareID, const std::string& name, const std::string& ip);
		void deleteHardware(controlID_t controlID);
		hardware::HardwareParams* getHardware(controlID_t controlID);

		// control
		const std::map<controlID_t,std::string> controlList() const;
		const std::map<protocol_t,std::string> protocolsOfControl(controlID_t) const;

		// loco
		datamodel::Loco* getLoco(const locoID_t locoID) const;
		const std::string& getLocoName(const locoID_t locoID);
		const std::map<locoID_t,datamodel::Loco*>& locoList() const;
		void locoSpeed(const managerID_t managerID, const protocol_t protocol, const address_t address, const speed_t speed);
		bool locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed);
		const speed_t locoSpeed(const locoID_t locoID) const;
		void locoDirection(const managerID_t managerID, const protocol_t protocol, const address_t address, const direction_t direction);
		void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction);
		void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on);
		void locoSave(const locoID_t locoID, const std::string& name, controlID_t& controlID, protocol_t& protocol, address_t& address);
		bool getProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// accessory
		void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state);
		const std::string& getAccessoryName(const accessoryID_t accessoryID);
		bool getAccessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		bool getSwitchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;

		// feedback
		datamodel::Feedback* getFeedback(feedbackID_t feedbackID);
		void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state);

		// block
		void block(const managerID_t managerID, const feedbackID_t feedbackID, const blockState_t);
		datamodel::Block* getBlock(const blockID_t blockID);
		const std::string& getBlockName(const blockID_t blockID);

		// switch
		const std::string& getSwitchName(const switchID_t switchID);

		// street
		datamodel::Street* getStreet(const streetID_t streetID);
		const std::string& getStreetName(const streetID_t streetID);

		// automode
		bool locoIntoBlock(const locoID_t locoID, const blockID_t blockID);
		bool locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID);
		bool locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID);
		bool locoStart(const locoID_t locoID);
		bool locoStop(const locoID_t locoID);
		bool locoStartAll();
		bool locoStopAll();
		bool autoMode;

	private:
		// const hardwareID_t hardwareOfControl(controlID_t controlID) const;

		// controllers (hardwareHandler & Webserver)
		std::vector<ManagerInterface*> controllers;

		// hardware (virt, CS2, ...)
		std::map<controlID_t,hardware::HardwareParams*> hardwareParams;
		mutable std::mutex hardwareMutex;

		// loco
		std::map<locoID_t,datamodel::Loco*> locos;
		mutable std::mutex locoMutex;

		// accessory
		std::map<accessoryID_t,datamodel::Accessory*> accessories;

		// feedback
		std::map<feedbackID_t,datamodel::Feedback*> feedbacks;
		std::mutex feedbackMutex;

		// block
		std::map<blockID_t,datamodel::Block*> blocks;
		std::mutex blockMutex;

		// switch
		std::map<switchID_t,datamodel::Switch*> switches;
		std::mutex switchMutex;

		// street
		std::map<streetID_t,datamodel::Street*> streets;
		std::mutex streetMutex;

		// storage
		storage::StorageHandler* storage;

		const std::string unknownLoco;
		const std::string unknownAccessory;
		const std::string unknownFeedback;
		const std::string unknownBlock;
		const std::string unknownSwitch;
		const std::string unknownStreet;
};

#endif // MANAGER_H


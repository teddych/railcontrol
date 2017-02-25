#ifndef MANAGER_H
#define MANAGER_H

#include <map>
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
		void booster(const managerID_t managerID, const boosterStatus_t status);
		void saveHardware(const controlID_t& controlID, const hardwareID_t& hardwareID, const std::string& name, const std::string& ip);
		void deleteHardware(controlID_t controlID);
		hardware::HardwareParams* getHardware(controlID_t controlID);
		bool getProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		bool getAccessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		const std::string& getLocoName(const locoID_t locoID);
		const std::string& getAccessoryName(const accessoryID_t accessoryID);
		void locoSpeed(const managerID_t managerID, const protocol_t protocol, const address_t address, const speed_t speed);
		void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed);
		void locoDirection(const managerID_t managerID, const protocol_t protocol, const address_t address, const direction_t direction);
		void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction);
		void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on);
		void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state);
		const std::map<locoID_t,datamodel::Loco*>& locoList() const;
		void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state);
		static void getAccessoryTexts(const accessoryState_t state, unsigned char& color, unsigned char& on, char*& colorText, char*& onText);
	private:
		std::vector<ManagerInterface*> controllers;
		std::map<controlID_t,hardware::HardwareParams*> hardwareParams;
		std::map<locoID_t,datamodel::Loco*> locos;
		std::map<accessoryID_t,datamodel::Accessory*> accessories;
		storage::StorageHandler* storage;
		const std::string unknownLoco;
		const std::string unknownAccessory;
};

#endif // MANAGER_H


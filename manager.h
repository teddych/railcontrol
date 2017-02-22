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
		void go(const managerID_t managerID);
		void stop(const managerID_t managerID);
		void saveHardware(const controlID_t& controlID, const hardwareID_t& hardwareID, const std::string& name, const std::string& ip);
		void deleteHardware(controlID_t controlID);
		hardware::HardwareParams* getHardware(controlID_t controlID);
		bool getProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		const std::string& getLocoName(const locoID_t locoID);
		void locoSpeed(const managerID_t managerID, const protocol_t protocol, const address_t address, const speed_t speed);
		void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed);
		void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on);
		const std::map<locoID_t,datamodel::Loco*>& locoList() const;
		void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state);
	private:
		std::vector<ManagerInterface*> controllers;
		std::map<controlID_t,hardware::HardwareParams*> hardwareParams;
		std::map<locoID_t,datamodel::Loco*> locos;
		storage::StorageHandler* storage;
		const std::string unknownLoco;
};

#endif // MANAGER_H


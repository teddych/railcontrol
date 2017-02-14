#ifndef MANAGER_H
#define MANAGER_H

#include <map>
#include <string>
#include <vector>

#include "config.h"
#include "datamodel/datamodel.h"
#include "manager_interface.h"
#include "storage/storage_handler.h"

class Manager {
	public:
    Manager(Config& config);
    ~Manager();
		void go(const managerID_t controlID);
		void stop(const managerID_t controlID);
    bool getProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const;
		void locoSpeed(const managerID_t controlID, const locoID_t locoID, const speed_t speed);
		const std::map<locoID_t,datamodel::Loco*>& locoList() const;
	private:
    std::vector<ManagerInterface*> controllers;
		std::map<locoID_t,datamodel::Loco*> locos;
		storage::StorageHandler* storage;
};

#endif // MANAGER_H


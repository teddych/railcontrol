#ifndef MANAGER_H
#define MANAGER_H

#include <map>
#include <vector>

#include "control.h"
#include "datamodel/datamodel.h"
#include "storage/storage_handler.h"

class Manager {
	public:
    Manager();
    ~Manager();
		void go(const controlID_t controlID);
		void stop(const controlID_t controlID);
    bool getProtocolAddress(const locoID_t locoID, hardwareControlID_t& hardwareControlID, protocol_t& protocol, address_t& address);
		void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed);
	private:
    std::vector<Control*> controllers;
		std::map<locoID_t,datamodel::Loco*> locos;
		storage::StorageHandler* storage;
};

#endif // MANAGER_H


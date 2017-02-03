#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "control.h"
#include "datamodel/datamodel.h"
#include "storage/storage.h"

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
		std::vector<datamodel::Loco*> locos;
		storage::Storage* storage;
};

#endif // MANAGER_H


#ifndef MANAGER_INTERFACE_H
#define MANAGER_INTERFACE_H

#include <string>

#include "datatypes.h"

class ManagerInterface {
	public:
		ManagerInterface() {};
    virtual ~ManagerInterface() {};
		virtual void go(const managerID_t managerID) = 0;
		virtual void stop(const managerID_t managerID) = 0;
		virtual void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) = 0;
  private:
    managerID_t managerID;
    std::string name;
};

#endif // MANAGER_INTERFACE_H


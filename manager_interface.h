#ifndef MANAGER_INTERFACE_H
#define MANAGER_INTERFACE_H

#include <string>

#include "datatypes.h"

class ManagerInterface {
	public:
		ManagerInterface() {};
    virtual ~ManagerInterface() {};
		virtual void booster(const managerID_t managerID, const boosterStatus_t status) = 0;
		virtual void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) = 0;
		virtual void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) = 0;
		virtual void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) = 0;
		virtual void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) = 0;
		virtual void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) = 0;
  private:
    managerID_t managerID;
    std::string name;
};

/*
inline bool ManagerInterface::operator==(const ManagerInterface& mi) {
	if (managerID == mi.managerID) {
		if (managerID == MANAGER_ID_HARDWARE) {
			hardware::HardwareHandler* hw1 = static_cast<HardwareHandler*>this;
			hardware::HardwareHandler* hw2 = static_cast<HardwareHandler*>(&mi);
			if (hw1.getControlID() == hw2.getControlID()) {
				return true;
			}
			return false;
		}
		return true;
	}
	return false;
}
*/

#endif // MANAGER_INTERFACE_H


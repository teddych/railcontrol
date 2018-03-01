#pragma once

#include <string>

#include "datatypes.h"

class ManagerInterface {
	public:
		ManagerInterface(managerID_t managerID);
    virtual ~ManagerInterface() {};
		const managerID_t getManagerID() const;
		virtual void booster(const managerID_t managerID, const boosterStatus_t status) = 0;
		virtual void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) = 0;
		virtual void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) = 0;
		virtual void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) = 0;
		virtual void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) = 0;
		virtual void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) = 0;
		virtual void block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) = 0;
		virtual void handleSwitch(const managerID_t managerID, const switchID_t switchID, const switchState_t state) = 0;
		virtual void locoIntoBlock(const locoID_t locoID, const blockID_t blockID) = 0;
		virtual void locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) = 0;
		virtual void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) = 0;
		virtual void locoStart(const locoID_t locoID) = 0;
		virtual void locoStop(const locoID_t locoID) = 0;

  private:
    managerID_t managerID;
    std::string name;
};

inline ManagerInterface::ManagerInterface(managerID_t managerID) :
	managerID(managerID) {
}

inline const managerID_t ManagerInterface::getManagerID() const {
	return managerID;
}


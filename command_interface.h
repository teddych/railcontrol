#pragma once

#include <string>
#include <vector>

#include "datatypes.h"

class CommandInterface {
	public:
		CommandInterface(controlType_t managerID);
		virtual ~CommandInterface() {};
		const controlType_t getManagerID() const { return managerID; }
		virtual const std::string getName() const = 0;
		virtual void booster(const controlType_t managerID, const boosterStatus_t status) = 0;
		virtual void locoSpeed(const controlType_t managerID, const locoID_t locoID, const speed_t speed) = 0;
		virtual void locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction) = 0;
		virtual void locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool on) = 0;
		virtual void accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) = 0;
		virtual void accessorySettings(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) {}
		virtual void accessoryDelete(const accessoryID_t accessoryID, const std::string& name) {}
		virtual void feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state) = 0;
		virtual void block(const controlType_t managerID, const blockID_t blockID, const lockState_t state) = 0;
		virtual void handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state, const bool on) = 0;
		virtual void switchSettings(const switchID_t switchID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const std::string rotation) {};
		virtual void locoIntoBlock(const locoID_t locoID, const blockID_t blockID) = 0;
		virtual void locoRelease(const locoID_t) = 0;
		virtual void blockRelease(const blockID_t) = 0;
		virtual void streetRelease(const streetID_t) = 0;
		virtual void locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) = 0;
		virtual void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) = 0;
		virtual void locoStart(const locoID_t locoID) = 0;
		virtual void locoStop(const locoID_t locoID) = 0;
		virtual void getProtocols(std::vector<protocol_t>& protocols) const {};
		virtual bool protocolSupported(protocol_t protocol) const { return false; };

	private:
		controlType_t managerID;
};

inline CommandInterface::CommandInterface(controlType_t managerID) :
	managerID(managerID) {
}


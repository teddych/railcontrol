#include "automode.h"

namespace automode {

	AutoMode::AutoMode(Manager& manager) :
		ManagerInterface(MANAGER_ID_AUTOMODE),
		manager(manager) {
		}

	AutoMode::~AutoMode() {
	}

	void AutoMode::booster(const managerID_t managerID, const boosterStatus_t status) {
	}

	void AutoMode::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
	}

	void AutoMode::locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) {
	}

	void AutoMode::locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool state) {
	}

	void AutoMode::accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) {
	}

	void AutoMode::feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) {
	}

	void AutoMode::block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) {
	}

	void AutoMode::handleSwitch(const managerID_t managerID, const switchID_t switchID, const switchState_t state) {
	}

}; // namespace automode

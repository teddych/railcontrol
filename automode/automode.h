#ifndef AUTOMODE_AUTOMODE_H
#define AUTOMODE_AUTOMODE_H

#include "manager_interface.h"
#include "manager.h"

namespace automode {

	class AutoMode : public ManagerInterface {
		public:
			AutoMode(Manager& manager);
			~AutoMode();
			void booster(const managerID_t managerID, const boosterStatus_t status) override;
			void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) override;
			void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) override;
			void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
			void block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) override;
			void handleSwitch(const managerID_t managerID, const switchID_t switchID, const switchState_t state) override;
		private:
			Manager& manager;
	};

}; // namespace webserver

#endif // AUTOMODE_AUTOMODE_H


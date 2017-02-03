#ifndef HARDWARE_HANDLER_H
#define HARDWARE_HANDLER_H

#include <string>
#include <vector>

#include "control.h"
#include "hardware_params.h"
#include "hardware_properties.h"
#include "manager.h"

class HardwareHandler : public Control {
	public:
		HardwareHandler(Manager& manager);
		~HardwareHandler();
		void go(const controlID_t controlID) override;
		void stop(const controlID_t controlID) override;
		void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) override;
		hardwareControlID_t getHardwareControlID(const locoID_t locoID);
	private:
		std::vector<HardwareProperties*> hardware;
		hardwareControlID_t nextHardwareControlID;
		Manager& manager;
};

#endif // HARDWARE_HANDLER_H


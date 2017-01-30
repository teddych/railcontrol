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
		//loco_id_t get_loco_id(protocol_t protocol, address_t address);
		//hardware_control_id_t get_hardware_control_id(loco_id_t loco_id);
		void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) override;
		hardwareControlID_t getHardwareControlID(const locoID_t locoID);
	private:
		std::vector<HardwareProperties*> hardware;
		hardwareControlID_t nextHardwareControlID;
		Manager& manager;
};

#endif // HARDWARE_HANDLER_H


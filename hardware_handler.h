#ifndef HARDWARE_HANDLER_H
#define HARDWARE_HANDLER_H

#include <vector>

#include "control.h"
#include "hardware_properties.h"
#include "manager.h"

class HardwareHandler : public Control {
	public:
		HardwareHandler(Manager& m);
		~HardwareHandler();
		//loco_id_t get_loco_id(protocol_t protocol, address_t address);
		//hardware_control_id_t get_hardware_control_id(loco_id_t loco_id);
		void locoSpeed(const controlID_t controlID, const protocol_t protocol, const address_t address, const speed_t speed);
		hardwareControlID_t getHardwareControlID(protocol_t protocol, address_t address);
	private:
		std::vector<HardwareProperties*> hardware;
		hardwareControlID_t nextHardwareControlID;
		Manager& m;
};

#endif // HARDWARE_HANDLER_H


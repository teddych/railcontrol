#ifndef HARDWARE_HANDLER_H
#define HARDWARE_HANDLER_H

#include <vector>

#include "control.h"
#include "hardware_properties.h"
#include "manager.h"

class hardware_handler : public control {
	public:
		hardware_handler(manager& m);
		~hardware_handler();
		//loco_id_t get_loco_id(protocol_t protocol, address_t address);
		//hardware_control_id_t get_hardware_control_id(loco_id_t loco_id);
		void loco_speed(const control_id_t control_id, const protocol_t protocol, const address_t address, const speed_t speed);
		hardware_control_id_t get_hardware_control_id(protocol_t protocol, address_t address);
	private:
		std::vector<hardware_properties*> hardware;
		hardware_control_id_t next_hardware_control_id;
		manager& m;
};

#endif // HARDWARE_HANDLER_H


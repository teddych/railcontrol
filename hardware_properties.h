#ifndef HARDWARE_PROPERTIES_H
#define HARDWARE_PROPERTIES_H

#include <string>

#include "control.h"
#include "hardware/hardware.h"

typedef unsigned char hardware_id_t;

enum hardware_ids {
	HARDWARE_ID_NONE = 0,
  HARDWARE_ID_VIRT,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

static std::string hardware_symbols[] = {
	"none",
	"virt",
	"cs2"
};

// the types of the class factories
typedef hardware::hardware* create_hardware_t();
typedef void destroy_hardware_t(hardware::hardware*);

class hardware_properties : public control {
	public:
		hardware_properties(const hardware_id_t hardware_id, const hardware_control_id_t hardware_control_id);
		~hardware_properties();
		std::string name() const;
		void start();
		void stop();
		void loco_speed(protocol_t protocol, address_t address, speed_t speed);
	private:
		hardware_id_t hardware_id;
		hardware_control_id_t hardware_control_id;
		create_hardware_t* create_hardware;
		destroy_hardware_t* destroy_hardware;
		hardware::hardware* instance;
		void* dlhandle;
};

inline void hardware_properties::loco_speed(protocol_t protocol, address_t address, speed_t speed) {
	instance->loco_speed(protocol, address, speed);
}

#endif // HARDWARE_PROPERTIES_H


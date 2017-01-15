#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <string>

#include "hardware/hardware.h"

typedef unsigned int hardware_id_t;

enum hardware_ids {
  HARDWARE_ID_VIRT = 0,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

static std::string hardware_symbols[] = {
	"virt",
	"cs2"
};

class dynamic {
	public:
		int symbols(hardware_id_t hardware_id, void** dlhandle, create_hardware_t** create_hardware, destroy_hardware_t** destroy_hardware);
};

#endif // DYNAMIC_H


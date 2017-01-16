#ifndef HARDWARE_PROPERTIES_H
#define HARDWARE_PROPERTIES_H

#include <string>

#include "hardware/hardware.h"

typedef unsigned char hardware_id_t;
typedef unsigned char control_id_t;

enum hardware_ids {
	HARDWARE_ID_NONE = 0,
  HARDWARE_ID_VIRT,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

static std::string hardware_symbols[] = {
	"virt",
	"cs2"
};

// the types of the class factories
typedef hardware::hardware* create_hardware_t();
typedef void destroy_hardware_t(hardware::hardware*);

class hardware_properties {
	public:
		hardware_properties(hardware_id_t hardware_id, control_id_t control_id);
		~hardware_properties();
		std::string name() const;
		int start();
		int stop();
	private:
		hardware_id_t hardware_id;
		control_id_t control_id;
		create_hardware_t* create_hardware;
		destroy_hardware_t* destroy_hardware;
		hardware::hardware* instance;
		void* dlhandle;
};

#endif // HARDWARE_PROPERTIES_H


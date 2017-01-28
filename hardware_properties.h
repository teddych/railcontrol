#ifndef HARDWARE_PROPERTIES_H
#define HARDWARE_PROPERTIES_H

#include <string>

#include "control.h"
#include "hardware/control_interface.h"

typedef unsigned char hardware_id_t;

enum hardwareIDs {
	HARDWARE_ID_NONE = 0,
  HARDWARE_ID_VIRT,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

static std::string hardwareSymbols[] = {
	"none",
	"virtual",
	"cs2"
};

// the types of the class factories
typedef hardware::ControlInterface* create_hardware_t();
typedef void destroy_hardware_t(hardware::ControlInterface*);

class HardwareProperties : public Control {
	public:
		HardwareProperties(const hardware_id_t hardwareID, const hardwareControlID_t hardwareControlID);
		~HardwareProperties();
		std::string name() const;
		void start();
		void stop();
		void locoSpeed(protocol_t protocol, address_t address, speed_t speed);
	private:
		hardware_id_t hardwareID;
		hardwareControlID_t hardwareControlID;
		create_hardware_t* createHardware;
		destroy_hardware_t* destroyHardware;
		hardware::ControlInterface* instance;
		void* dlhandle;
};

inline void HardwareProperties::locoSpeed(protocol_t protocol, address_t address, speed_t speed) {
	instance->locoSpeed(protocol, address, speed);
}

#endif // HARDWARE_PROPERTIES_H


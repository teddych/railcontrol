#ifndef HARDWARE_PROPERTIES_H
#define HARDWARE_PROPERTIES_H

#include <string>

#include "datatypes.h"
#include "control.h"
#include "hardware/control_interface.h"
#include "hardware_params.h"
#include "util.h"

// the types of the class factories
typedef hardware::ControlInterface* createHardware_t(struct Params params);
typedef void destroyHardware_t(hardware::ControlInterface*);

class HardwareProperties : public Control {
	public:
		HardwareProperties(const hardware_id_t hardwareID, const hardwareControlID_t hardwareControlID, const struct Params& params);
		~HardwareProperties();
		std::string getName() const;
		void go(const controlID_t controlID) override;
		void stop(const controlID_t controlID) override;
		void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) override;
		hardwareControlID_t getHardwareControlID(const locoID_t locoID);
	private:
		hardware_id_t hardwareID;
		hardwareControlID_t hardwareControlID;
		createHardware_t* createHardware;
		destroyHardware_t* destroyHardware;
		hardware::ControlInterface* instance;
		void* dlhandle;
		struct Params params;
};

#endif // HARDWARE_PROPERTIES_H


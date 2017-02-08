#ifndef HARDWARE_PROPERTIES_H
#define HARDWARE_PROPERTIES_H

#include <string>

#include "datatypes.h"
#include "control.h"
#include "hardware_interface.h"
#include "hardware_params.h"
#include "util.h"

namespace hardware {

	// the types of the class factories
	typedef hardware::HardwareInterface* createHardware_t(struct HardwareParams params);
	typedef void destroyHardware_t(hardware::HardwareInterface*);

	class HardwareHandler: public Control {
		public:
			HardwareHandler(const hardware_id_t hardwareID, const hardwareControlID_t hardwareControlID, const struct HardwareParams& params);
			~HardwareHandler();
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
			hardware::HardwareInterface* instance;
			void* dlhandle;
			struct HardwareParams params;
	};

}
;
// namespace hardware

#endif // HARDWARE_PROPERTIES_H


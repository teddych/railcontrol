#pragma once

#include <string>

#include "datatypes.h"
#include "hardware_interface.h"
#include "hardware_params.h"
#include "manager.h"
#include "manager_interface.h"
#include "util.h"

namespace hardware {

	// the types of the class factories
	typedef hardware::HardwareInterface* createHardware_t(const hardware::HardwareParams* params);
	typedef void destroyHardware_t(hardware::HardwareInterface*);

	class HardwareHandler: public ManagerInterface {
		public:
			HardwareHandler(Manager& manager, const HardwareParams* params);
			~HardwareHandler();
			controlID_t getControlID() const { return params->controlID; }
			const std::string getName() const override;

			void booster(const managerID_t managerID, boosterStatus_t status) override;
			void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) override;
			void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) override;
			void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) override {};
			void block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) override {};
			void handleSwitch(const managerID_t managerID, const switchID_t switchID, const switchState_t state) override;
			void locoIntoBlock(const locoID_t locoID, const blockID_t blockID) override {};
			void locoRelease(const locoID_t) override {};
			void blockRelease(const blockID_t) override {};
			void streetRelease(const streetID_t) override {};
			void locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) override {};
			void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) override {};
			void locoStart(const locoID_t locoID) override {};
			void locoStop(const locoID_t locoID) override {};
			void getProtocols(std::vector<protocol_t>& protocols) const override;
			bool protocolSupported(protocol_t protocol) const override;
		private:
			Manager& manager;
			createHardware_t* createHardware;
			destroyHardware_t* destroyHardware;
			hardware::HardwareInterface* instance;
			const HardwareParams* params;
	};

}; // namespace hardware


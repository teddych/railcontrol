#pragma once

#include <string>

#include "ControlInterface.h"
#include "datatypes.h"
#include "hardware/HardwareInterface.h"
#include "hardware/HardwareParams.h"
#include "Manager.h"

namespace hardware {

	// the types of the class factories
	typedef hardware::HardwareInterface* createHardware_t(const hardware::HardwareParams* params);
	typedef void destroyHardware_t(hardware::HardwareInterface*);

	class HardwareHandler: public ControlInterface {
		public:
			HardwareHandler(Manager& manager, const HardwareParams* params);
			~HardwareHandler();
			controlID_t ControlID() const { return params->controlID; }
			const std::string GetName() const override;

			void AccessoryProtocols(std::vector<protocol_t>& protocols) const override;
			bool AccessoryProtocolSupported(protocol_t protocol) const override;
			void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void ArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const override;
			void Booster(const controlType_t controlType, boosterState_t status) override;
			bool CanHandleAccessories() const override;
			bool CanHandleFeedback() const override;
			bool CanHandleLocos() const override;
			void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction) override;
			void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on) override;
			void LocoProtocols(std::vector<protocol_t>& protocols) const override;
			bool LocoProtocolSupported(protocol_t protocol) const override;
			void LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed) override;
			void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on) override;
			void SignalState(const controlType_t controlType, const signalID_t signalID, const signalState_t state, const bool on) override;

		private:
			Manager& manager;
			createHardware_t* createHardware;
			destroyHardware_t* destroyHardware;
			hardware::HardwareInterface* instance;
			const HardwareParams* params;

			static const std::string hardwareSymbols[];
	};

}; // namespace hardware


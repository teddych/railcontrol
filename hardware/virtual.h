#pragma once

#include <cstring>

#include "hardware/HardwareInterface.h"
#include "hardware/HardwareParams.h"
#include "Logger/Logger.h"

namespace hardware
{

	class Virtual : HardwareInterface
	{
		public:
			// Constructor
			Virtual(const HardwareParams* params);

			// name() must be implemented
			const std::string GetName() const override { return name; };

			// turn booster on or off
			void Booster(const boosterStatus_t status) override;

			// set loco speed
			void SetLocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed) override;

			// set loco direction
			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;

			// set loco function
			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;

			// accessory command
			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			std::string name;
			Logger::Logger* logger;
	};

	extern "C" Virtual* create_virtual(const HardwareParams* params);
	extern "C" void destroy_virtual(Virtual* virt);

} // namespace


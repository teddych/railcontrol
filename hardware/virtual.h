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
			Virtual(const HardwareParams* params);

			bool CanHandleLocos() const { return true; }
			bool CanHandleAccessories() const { return true; }
			bool CanHandleFeedback() const { return true; }

			void Booster(const boosterState_t status) override;

			void LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed) override;

			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;

			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;

			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			Logger::Logger* logger;
			std::string name;
	};

	extern "C" Virtual* create_virtual(const HardwareParams* params);
	extern "C" void destroy_virtual(Virtual* virt);

} // namespace


#pragma once

#include <cstring>

#include "Hardware/HardwareInterface.h"
#include "Hardware/HardwareParams.h"
#include "Logger/Logger.h"

namespace Hardware
{

	class Virtual : HardwareInterface
	{
		public:
			Virtual(const HardwareParams* params);

			bool CanHandleLocos() const override { return true; }
			bool CanHandleAccessories() const override { return true; }
			bool CanHandleFeedback() const override { return true; }

			void Booster(const boosterState_t status) override;

			void LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed) override;

			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;

			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;

			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			Logger::Logger* logger;
	};

	extern "C" Virtual* create_virtual(const HardwareParams* params);
	extern "C" void destroy_virtual(Virtual* virt);

} // namespace


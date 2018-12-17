#pragma once

#include <string>

#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "manager.h"

namespace hardware
{
	class M6051 : HardwareInterface
	{
		public:
			M6051(const HardwareParams* params);
			~M6051();
			const std::string GetName() const override { return name; };
			void GetProtocols(std::vector<protocol_t>& protocols) const override;
			virtual bool ProtocolSupported(protocol_t protocol) const override;
			void Booster(const boosterStatus_t status) override;
			void SetLocoSpeed(const protocol_t& protocol, const address_t& address, const LocoSpeed& speed) override;
			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;
			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			std::string name;
			Manager* manager;
	};

	extern "C" M6051* create_m6051(const HardwareParams* params);
	extern "C" void destroy_m6051(M6051* m6051);

} // namespace


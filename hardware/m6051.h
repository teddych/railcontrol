#pragma once

#include <mutex>
#include <string>
#include <unistd.h>   //close & write;

#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "Logger/Logger.h"
#include "manager.h"

namespace hardware
{
	class M6051 : HardwareInterface
	{
		public:
			M6051(const HardwareParams* params);
			~M6051();

			bool CanHandleLocos() const { return true; }
			bool CanHandleAccessories() const { return true; }
			bool CanHandleFeedback() const { return true; }

			void GetLocoProtocols(std::vector<protocol_t>& protocols) const override { protocols.push_back(ProtocolMM2); }

			bool LocoProtocolSupported(protocol_t protocol) const override { return (protocol == ProtocolMM2); }

			void GetAccessoryProtocols(std::vector<protocol_t>& protocols) const override { protocols.push_back(ProtocolMM2); }

			bool AccessoryProtocolSupported(protocol_t protocol) const override { return (protocol == ProtocolMM2); }

			void GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const override
			{
				argumentTypes[1] = SerialPort;
				argumentTypes[2] = S88Modules;
			}

			void Booster(const boosterState_t status) override;
			void LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed) override;
			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;
			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			Logger::Logger* logger;
			static const unsigned char MaxS88Modules = 62;
			std::string name;
			Manager* manager;
			int ttyFileDescriptor;
			mutable std::mutex ttyMutex;
			volatile bool run;
			unsigned char s88Modules;
			std::thread s88Thread;
			unsigned char s88Memory[MaxS88Modules];
			std::map<address_t, unsigned char> speedMap;
			std::map<address_t, unsigned char> functionMap;

			unsigned char GetSpeedMapEntry(address_t address)
			{
				return speedMap.count(address) == 0 ? 0 : speedMap[address];
			}

			unsigned char GetFunctionMapEntry(address_t address)
			{
				return functionMap.count(address) == 0 ? 0 : functionMap[address];
			}

			void SendOneByte(unsigned char byte)
			{
				std::lock_guard<std::mutex> Guard(ttyMutex);
				__attribute__((unused)) int ret = write(ttyFileDescriptor, &byte, 1);
			}

			void SendTwoBytes(unsigned char byte1, unsigned char byte2)
			{
				std::lock_guard<std::mutex> Guard(ttyMutex);
				__attribute__((unused)) int ret = write(ttyFileDescriptor, &byte1, 1);
				ret = write(ttyFileDescriptor, &byte2, 1);
			}

			void S88Worker();
	};

	extern "C" M6051* create_m6051(const HardwareParams* params);
	extern "C" void destroy_m6051(M6051* m6051);

} // namespace


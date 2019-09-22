#pragma once

#include <mutex>
#include <string>
#include <unistd.h>   //close & write;

#include "HardwareInterface.h"
#include "HardwareParams.h"
#include "Logger/Logger.h"
#include "network/Serial.h"

namespace hardware
{
	class OpenDcc : HardwareInterface
	{
		public:
			OpenDcc(const HardwareParams* params);
			~OpenDcc();

			bool CanHandleLocos() const override { return true; }
			bool CanHandleAccessories() const override { return true; }
			bool CanHandleFeedback() const override { return true; }

			void GetLocoProtocols(std::vector<protocol_t>& protocols) const override { protocols.push_back(ProtocolDCC); }

			bool LocoProtocolSupported(protocol_t protocol) const override { return (protocol == ProtocolDCC); }

			void GetAccessoryProtocols(std::vector<protocol_t>& protocols) const override { protocols.push_back(ProtocolDCC); }

			bool AccessoryProtocolSupported(protocol_t protocol) const override { return (protocol == ProtocolDCC); }

			void GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const override
			{
				argumentTypes[1] = SerialPort;
				argumentTypes[2] = S88Modules;
				argumentTypes[3] = S88Modules;
				argumentTypes[4] = S88Modules;
			}

			void Booster(const boosterState_t status) override;
			void LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed) override;
			void LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) override;
			void LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) override;
			void Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on) override;

		private:
			enum Commands : unsigned char
			{
				XNop = 0xC4,
				XPwrOn = 0xA7,
				XPwrOff = 0xA6,
				XLok = 0x80,
				XFunc = 0x88,
				XFunc2 = 0x89,
				XFunc34 = 0x8A,
				XTrnt = 0x90
			};
			enum Answers : unsigned char
			{
				OK = 0x00,
				XBADPRM = 0x02,
				XPWOFF = 0x06,
				XNODATA = 0x0A,
				XNOSLOT = 0x0B,
				XLOWTSP = 0x40,
				XLKHALT = 0x41,
				XLKPOFF = 0x42
			};
			static const unsigned char MaxS88Modules = 128;
			static const unsigned char MaxLocoFunctions = 28;
			static const unsigned short MaxLocoAddress = 10239;
			static const unsigned short MaxAccessoryAddress = 2043;

			Logger::Logger* logger;
			Network::Serial serialLine;
			volatile bool run;
			unsigned char s88Modules1;
			unsigned char s88Modules2;
			unsigned char s88Modules3;
			unsigned short s88Modules;

			std::thread s88Thread;
			unsigned char s88Memory[MaxS88Modules];
			std::map<address_t, uint16_t> cacheBasic;
			std::map<address_t, uint32_t> cacheFunctions;

			uint16_t GetCacheBasicEntry(const address_t address) { return cacheBasic.count(address) == 0 ? 0 : cacheBasic[address]; }
			uint32_t GetCacheFunctionsEntry(const address_t address) { return cacheFunctions.count(address) == 0 ? 0 : cacheFunctions[address]; }

			bool CheckLocoAddress(const address_t address) { return 0 < address && address <= MaxLocoAddress; }
			bool CheckAccessoryAddress(const address_t address) { return 0 < address && address <= MaxAccessoryAddress; }

			bool SendP50XOnly();
			bool SendOneByteCommand(const unsigned char data);
			bool SendNop() { return SendOneByteCommand(XNop); }
			bool SendPowerOn() { return SendOneByteCommand(XPwrOn); }
			bool SendPowerOff() { return SendOneByteCommand(XPwrOff); }
			bool SendXLok(const address_t address, const unsigned char speed, const unsigned char direction);
			bool SendXFunc(const address_t address, const unsigned char functions);
			bool SendXFunc2(const address_t address, const unsigned char functions);
			bool SendXFunc34(const address_t address, const unsigned char functions3, const unsigned char functions4);
			bool ReceiveFunctionCommandAnswer();

			void S88Worker();
	};

	extern "C" OpenDcc* create_opendcc(const HardwareParams* params);
	extern "C" void destroy_opendcc(OpenDcc* opendcc);

} // namespace


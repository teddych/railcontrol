#include <cstring>    //memset
#include <fcntl.h>
#include <sstream>
#include <termios.h>

#include "text/converters.h"
#include "hardware/m6051.h"
#include "util.h"

namespace hardware
{

	// create instance of m6051
	extern "C" M6051* create_m6051(const HardwareParams* params)
	{
		return new M6051(params);
	}

	// delete instance of m6051
	extern "C" void destroy_m6051(M6051* m6051)
	{
		delete(m6051);
	}

	M6051::M6051(const HardwareParams* params) :
		manager(params->manager),
		logger(Logger::Logger::GetLogger("M6051 " + params->name + " " + params->arg1))
	{
		std::stringstream ss;
		ss << "Maerklin Interface (6050/6051) / " << params->name << " at serial port " << params->arg1;
		name = ss.str();
		logger->Info(name);

		ttyFileDescriptor = open(params->arg1.c_str(), O_RDWR | O_NOCTTY);
		if (ttyFileDescriptor == -1)
		{
			logger->Error("unable to open {0}", params->arg1);
			return;
		}

		struct termios options;
		tcgetattr(ttyFileDescriptor, &options);
		cfsetispeed(&options, B2400);
		cfsetospeed(&options, B2400);
		options.c_cflag &= ~PARENB; // no parity
		options.c_cflag |= CSTOPB; // 2 stop bit
		options.c_cflag &= ~CSIZE;  // no datasize
		options.c_cflag |= CRTSCTS;  // hardware flow control
		options.c_cflag |= CS8;     // 8 data bits
		options.c_cflag |= CLOCAL;  // ignore control lines
		options.c_cflag |= CREAD;   // enable receiver
		options.c_cc[VMIN] = 1;     // read one byte at least
		options.c_cc[VTIME] = 10;    // timeout = 0.1s
		tcsetattr(ttyFileDescriptor, TCSANOW, &options); // store options
		tcflush(ttyFileDescriptor, TCIFLUSH); // clear RX buffer
	}

	M6051::~M6051()
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}
		close(ttyFileDescriptor);
	}

	void M6051::GetProtocols(std::vector<protocol_t>& protocols) const
	{
		protocols.push_back(ProtocolMM2);
	}

	bool M6051::ProtocolSupported(protocol_t protocol) const
	{
		return (protocol == ProtocolMM2);
	}

	void M6051::Booster(const boosterStatus_t status)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}

		unsigned char c;

		if (status)
		{
			logger->Info("Turning booster on");
			c = 96;
		}
		else
		{
			logger->Info("Turning booster off");
			c = 97;
		}
		SendOneByte(c);
	}

	void M6051::SetLocoSpeed(__attribute__((unused)) const protocol_t& protocol, const address_t& address, const LocoSpeed& speed)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}
		unsigned char speedMM = (speed / 69) + (GetSpeedMapEntry(address) & 16);
		speedMap[address] = speedMM;
		unsigned char addressMM = static_cast<unsigned char>(address);
		logger->Info("Setting speed of loco {0} to speed {1}", address, speedMM);
		SendTwoBytes(speedMM, addressMM);
	}

	void M6051::LocoDirection(__attribute__((unused)) const protocol_t& protocol, const address_t& address, __attribute__((unused)) const direction_t& direction)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}
		logger->Info("Changing direction of loco {0}", address);
		unsigned char speedMM = 15 + (GetSpeedMapEntry(address) & 16);
		unsigned char addressMM = static_cast<unsigned char>(address);
		SendTwoBytes(speedMM, addressMM);
	}

	void M6051::LocoFunction(__attribute__((unused)) const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		if (function > 4)
		{
			return;
		}

		if (ttyFileDescriptor < 0)
		{
			return;
		}

		logger->Info("Setting f{0} of loco {1} to \"{2}\"", function, address, on ? "on" : "off");
		unsigned char addressMM = static_cast<unsigned char>(address);
		if (function == 0)
		{
			unsigned char speedMM = (GetSpeedMapEntry(address) & 15) + (static_cast<unsigned char>(on) << 4);
			speedMap[address] = speedMM;
			SendTwoBytes(speedMM, addressMM);
			return;
		}

		unsigned char functionMM = GetFunctionMapEntry(address);
		unsigned char position = function - 1;
		functionMM &= (~(1 << position)); // mast out related function
		functionMM |= (static_cast<unsigned char>(on) << position); // add related function
		functionMap[address] = functionMM;
		functionMM += 64;
		SendTwoBytes(functionMM, addressMM);
	}

	void M6051::Accessory(__attribute__((unused)) const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}

		std::string stateText;
		text::Converters::accessoryStatus(state, stateText);
		logger->Info("Setting state of accessory {0}/{1} to \"{2}\"", address, stateText, on ? "on" : "off");
		unsigned char stateMM = (state == AccessoryStateOn ? 33 : 34);
		unsigned char addressMM = static_cast<unsigned char>(address);
		SendTwoBytes(stateMM, addressMM);
	}
} // namespace

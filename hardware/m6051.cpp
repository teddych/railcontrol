#include <cstring>    //memset
#include <fcntl.h>
#include <sstream>
#include <termios.h>
#include <unistd.h>   //close;

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

	// start the thing
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

	// stop the thing
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

	// turn booster on or off
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
		__attribute__((unused)) int ret = write(ttyFileDescriptor, &c, 1);
	}

	// set the speed of a loco
	void M6051::SetLocoSpeed(__attribute__((unused)) const protocol_t& protocol, const address_t& address, const LocoSpeed& speed)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}
		unsigned char speedMM = speed / 69;
		unsigned char addressMM = static_cast<unsigned char>(address);
		logger->Info("Setting speed of loco {0} to speed {1}", address, speedMM);
		__attribute__((unused)) int ret = write(ttyFileDescriptor, &speedMM, 1);
		ret = write(ttyFileDescriptor, &addressMM, 1);
	}

	// set the direction of a loco
	void M6051::LocoDirection(__attribute__((unused)) const protocol_t& protocol, const address_t& address, __attribute__((unused)) const direction_t& direction)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}
		logger->Info("Changing direction of loco {0}", address);
		unsigned char speedMM = 15;
		unsigned char addressMM = static_cast<unsigned char>(address);
		__attribute__((unused)) int ret = write(ttyFileDescriptor, &speedMM, 1);
		ret = write(ttyFileDescriptor, &addressMM, 1);
	}

	// set loco function
	void M6051::LocoFunction(__attribute__((unused)) const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}
		logger->Info("Setting f%i of loco {0} to \"{1}\"", static_cast<int>(function), static_cast<int>(address), on ? "on" : "off");
	}

	void M6051::Accessory(__attribute__((unused)) const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		if (ttyFileDescriptor < 0)
		{
			return;
		}

		std::string stateText;
		text::Converters::accessoryStatus(state, stateText);
		logger->Info("Setting state of accessory {0}/{1} to \"{2}\"", static_cast<int>(address), stateText, on ? "on" : "off");
		unsigned char stateMM = (state == AccessoryStateOn ? 33 : 34);
		unsigned char addressMM = static_cast<unsigned char>(address);
		__attribute__((unused)) int ret = write(ttyFileDescriptor, &stateMM, 1);
		ret = write(ttyFileDescriptor, &addressMM, 1);
	}
} // namespace

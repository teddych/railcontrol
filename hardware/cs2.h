#ifndef HARDWARE_CS2_H
#define HARDWARE_CS2_H

#include <cstring>
#include <string>

#include "control_interface.h"

namespace hardware {

  class CS2 : ControlInterface {
    public:
			CS2(std::string& name2);
      int start(struct params &params) override;
      int stop() override;
			std::string getName() const override;
			std::string locoSpeed(protocol_t protocol, address_t address, speed_t speed) override;
    private:
			void sendCommand(const int sock, const struct sockaddr* sockaddr_in, const unsigned char prio, const unsigned char command, const unsigned char response, const unsigned char length, const char* data);
      void receiver();
      void sender();
      volatile unsigned char run;
      std::string name;
  };

} // namespace

#endif // HARDWARE_CS2_H


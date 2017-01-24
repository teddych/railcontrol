#ifndef HARDWARE_CS2_H
#define HARDWARE_CS2_H

#include <cstring>

#include "hardware.h"

namespace hardware {

  class cs2 : hardware {
    public:
      int start(struct params &params) override;
      int stop() override;
      std::string name() const override;
			void loco_speed(unsigned char protocol, unsigned short address, int speed);
    private:
			void send_command(const int sock, const struct sockaddr* sockaddr_in, const unsigned char prio, const unsigned char command, const unsigned char response, const unsigned char length, const char* data);
      void receiver();
      void sender();
      volatile unsigned char run;
  };

} // namespace

#endif // HARDWARE_CS2_H


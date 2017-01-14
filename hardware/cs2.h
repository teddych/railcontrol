#ifndef HARDWARE_CS2_H
#define HARDWARE_CS2_H

#include <cstring>

#include "hardware.h"

namespace hardware {

  class cs2 : public hardware::hardware {
    public:
      int start(struct params &params) override;
      int stop() override;
      std::string name() const override;
    private:
      void receiver();
      void sender();
      volatile unsigned char run;
  };

} // namespace

#endif // HARDWARE_CS2_H


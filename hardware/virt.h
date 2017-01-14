#ifndef HARDWARE_VIRTUAL_H
#define HARDWARE_VIRTUAL_H

#include <cstring>

#include "hardware.h"

namespace hardware {

  class virt : hardware {
    public:
      int start(struct params &params) override;
      int stop() override;
      std::string name() const override;
  };

} // namespace

#endif // HARDWARE_VIRTUAL_H


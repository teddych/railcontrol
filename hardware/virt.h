#ifndef HARDWARE_VIRTUAL_H
#define HARDWARE_VIRTUAL_H

#include <cstring>

#include "hardware.h"

namespace hardware {

  class virt : hardware {
    public:
      int start(struct params &params);
      int stop();
      std::string name();
  };

} // namespace

#endif // HARDWARE_VIRTUAL_H


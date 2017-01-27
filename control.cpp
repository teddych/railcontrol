#include "control.h"
#include "util.h"

control::control(control_id_t control_id) :
  control_id(control_id) {
}

void control::loco_speed(const control_id_t control_id, const protocol_t protocol, const address_t address, const speed_t speed) {
  xlog("Non implemented loco_speed in control %u", control_id);
}


#include <iostream>

#include "manager.h"
#include "hardware_handler.h"
#include "util.h"
#include "webserver.h"

manager::manager() {
  controllers.push_back(new webserver(*this, 8080));
	controllers.push_back(new hardware_handler(*this));
}

manager::~manager() {
  for (auto control : controllers) {
    delete control;
  }
}

void manager::loco_speed(const control_id_t control_id, const loco_id_t loco_id, const speed_t speed) {
  for (auto control : controllers) {
    control->loco_speed(control_id, loco_id, speed);
  }
}


#include "control.h"

#include "util.h"

Control::Control(controlID_t controlID) :
  controlID(controlID) {
}

void Control::locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) {
  xlog("Non implemented loco_speed in control %u", controlID);
}


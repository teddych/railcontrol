#ifndef CONTROL_H
#define CONTROL_H

#include <string>

#include "datatypes.h"

class Control {
  public:
    Control(controlID_t control_id);
    virtual ~Control() {};
    controlID_t getControlID();
		virtual void go(const controlID_t controlID) = 0;
		virtual void stop(const controlID_t controlID) = 0;
		virtual void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) = 0;
  private:
    controlID_t controlID;
    std::string name;
};

inline controlID_t Control::getControlID() {
  return controlID;
}

#endif // CONTROL_H


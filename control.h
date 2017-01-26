#ifndef CONTROL_H
#define CONTROL_H

enum control_ids {
  CONTROL_ID_CONSOLE = 0,
  CONTROL_ID_HARDWARE,
  CONTROL_ID_WEBSERVER
};

class control {
  public:
    control(unsigned int control_id);
    unsigned int get_control_id();
		virtual void loco_speed(unsigned char protocol, unsigned short address, int speed) = 0;
  private:
    unsigned int control_id;
};

inline unsigned int control::get_control_id() {
  return control_id;
}

#endif // CONTROL_H


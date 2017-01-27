#ifndef CONTROL_H
#define CONTROL_H

typedef unsigned char control_id_t;
typedef unsigned char hardware_control_id_t;
typedef unsigned char protocol_t;
typedef unsigned short address_t;
typedef short speed_t;
typedef unsigned short loco_id_t;

enum control_ids : control_id_t {
  CONTROL_ID_CONSOLE = 0,
  CONTROL_ID_HARDWARE,
  CONTROL_ID_WEBSERVER
};

class control {
  public:
    control(control_id_t control_id);
    virtual ~control() {};
    control_id_t get_control_id();
		virtual void loco_speed(const control_id_t control_id, const loco_id_t, const speed_t speed);
  private:
    control_id_t control_id;
};

inline control_id_t control::get_control_id() {
  return control_id;
}

#endif // CONTROL_H


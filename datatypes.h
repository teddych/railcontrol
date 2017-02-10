#ifndef DATATYPES_H
#define DATATYPES_H

#include <string>

typedef unsigned char controlID_t;
typedef unsigned char hardwareControlID_t;
typedef unsigned char hardware_id_t;
typedef unsigned char protocol_t;
typedef unsigned short address_t;
typedef short speed_t;
typedef unsigned short locoID_t;

enum controlIDs : controlID_t {
  CONTROL_ID_CONSOLE = 0,
  CONTROL_ID_HARDWARE,
  CONTROL_ID_WEBSERVER
};

enum protocols : protocol_t {
	PROTOCOL_NONE = 0,
	PROTOCOL_ALL,
	PROTOCOL_MM1,
	PROTOCOL_MM2,
	PROTOCOL_MFX,
	PROTOCOL_DCC,
	PROTOCOL_SX1,
	PROTOCOL_SX2
};

enum hardwareIDs : hardware_id_t {
	HARDWARE_ID_NONE = 0,
  HARDWARE_ID_VIRT,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

#define ADDRESS_NONE 0

static std::string hardwareSymbols[] = {
	"none",
	"virtual",
	"cs2"
};

#endif // DATATYPES_H


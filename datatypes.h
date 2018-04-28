#pragma once

#include <string>

// common
typedef unsigned char managerID_t;
typedef unsigned char controlID_t;
typedef unsigned char hardwareType_t;
// booster
typedef unsigned char boosterStatus_t;

// objects in db
typedef unsigned short objectID_t;
// loco
typedef objectID_t locoID_t;
typedef unsigned char protocol_t;
typedef unsigned short address_t;
typedef unsigned char addressType_t;
typedef unsigned short speed_t;
typedef bool direction_t;
typedef unsigned char function_t;
// layoutItem
typedef unsigned char layoutRotation_t;
typedef unsigned char layoutItemSize_t;
typedef unsigned char layoutPosition_t;
// accessory
typedef objectID_t accessoryID_t;
typedef unsigned char accessoryType_t;
typedef unsigned char accessoryState_t;
typedef unsigned char accessoryColor_t;
typedef unsigned short accessoryTimeout_t;
// feedback
typedef objectID_t feedbackID_t;
typedef unsigned int feedbackPin_t;
typedef unsigned char feedbackState_t;
// block
typedef objectID_t blockID_t;
typedef unsigned char blockState_t;
// switch
typedef accessoryID_t switchID_t;
typedef accessoryType_t switchType_t;
typedef accessoryState_t switchState_t;
typedef accessoryTimeout_t switchTimeout_t;
// street
typedef objectID_t streetID_t;
typedef unsigned char streetState_t;

// relations in db
typedef unsigned short relationID_t;

// automode
typedef unsigned char locoState_t;

#define CONTROL_NONE 0
#define ADDRESS_NONE 0
#define LOCO_NONE 0
#define ACCESSORY_NONE 0
#define FEEDBACK_NONE 0
#define BLOCK_NONE 0
#define SWITCH_NONE 0
#define STREET_NONE 0

#define MAX_SPEED 1023

#define WIDTH_1 1
#define HEIGHT_1 1

enum managerIDs : managerID_t {
	MANAGER_ID_HARDWARE = 0,
	MANAGER_ID_AUTOMODE,
	MANAGER_ID_CONSOLE,
	MANAGER_ID_WEBSERVER
};

enum controlIDs : controlID_t {
	CONTROL_ID_NONE = 0,
	CONTROL_ID_CONSOLE,
	CONTROL_ID_WEBSERVER,
	CONTROL_ID_FIRST_HARDWARE = 10
};

enum boosterStatus : boosterStatus_t {
	BOOSTER_STOP = 0,
	BOOSTER_GO
};

enum protocols : protocol_t {
	PROTOCOL_NONE = 0,
	PROTOCOL_SERVER,
	PROTOCOL_MM1,
	PROTOCOL_MM2,
	PROTOCOL_MFX,
	PROTOCOL_DCC,
	PROTOCOL_DCC_SHORT,
	PROTOCOL_DCC_LONG,
	PROTOCOL_SX1,
	PROTOCOL_SX2,
	PROTOCOL_END = PROTOCOL_SX2
};

static std::string protocolSymbols[] = {
	"none",
	"all",
	"MM1",
	"MM2",
	"mfx",
	"DCC",
	"SX1",
	"SX2"
};

enum addressTypes : addressType_t {
	ADDRESS_TYPE_LOCO = 0,
	ADDRESS_TYPE_ACCESSORY
};

enum hardwareTypes : hardwareType_t {
	HARDWARE_TYPE_NONE = 0,
	HARDWARE_TYPE_VIRT,
	HARDWARE_TYPE_CS2,
	HARDWARE_TYPE_NUM
};

static std::string hardwareSymbols[] = {
	"none",
	"virtual",
	"cs2"
};

enum rotations : layoutRotation_t {
	ROTATION_0 = 0,
	ROTATION_90,
	ROTATION_180,
	ROTATION_270
};

typedef unsigned char objectType_t;

enum objectType : objectType_t {
	OBJECT_TYPE_LOCO = 1,
	OBJECT_TYPE_BLOCK,
	OBJECT_TYPE_FEEDBACK,
	OBJECT_TYPE_ACCESSORY,
	OBJECT_TYPE_SWITCH,
	OBJECT_TYPE_STREET
};

typedef unsigned char relationType_t;

enum relationType : relationType_t {
	RELATION_TYPE_BLOCK_STREET = 0,
	RELATION_TYPE_STREET_FEEDBACK
};

enum accessoryColor : accessoryColor_t {
	ACCESSORY_COLOR_RED = 0,
	ACCESSORY_COLOR_GREEN,
	ACCESSORY_COLOR_YELLOW,
	ACCESSORY_COLOR_WHITE
};

enum accessoryType : accessoryType_t {
	ACCESSORY_TYPE_DEFAULT = 0
};

enum accessoryState : accessoryState_t {
	ACCESSORY_STATE_OFF = 0,
	ACCESSORY_STATE_ON
};

enum feedbackState : feedbackState_t {
	FEEDBACK_STATE_FREE = 0,
	FEEDBACK_STATE_OCCUPIED
};

enum blockState : blockState_t {
	BLOCK_STATE_FREE = 0,
	BLOCK_STATE_RESERVED,
	BLOCK_STATE_LOCKED
};

enum directionState : direction_t {
	DIRECTION_LEFT = false,
	DIRECTION_RIGHT = true
};

enum streetState : streetState_t {
	STREET_STATE_FREE = 0,
	STREET_STATE_RESERVED,
	STREET_STATE_LOCKED
};

enum switchType : switchType_t {
	SWITCH_LEFT = 0,
	SWITCH_RIGHT
};

enum switchState : switchState_t {
	SWITCH_STRAIGHT = 0,
	SWITCH_TURNOUT
};

enum locoState : locoState_t {
	LOCO_STATE_MANUAL = 0,
	LOCO_STATE_OFF,
	LOCO_STATE_SEARCHING,
	LOCO_STATE_RUNNING,
	LOCO_STATE_STOPPING,
	LOCO_STATE_ERROR
};


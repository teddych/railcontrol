#ifndef DATATYPES_H
#define DATATYPES_H

#include <string>

// common
typedef unsigned char managerID_t;
typedef unsigned char controlID_t;
typedef unsigned char hardwareID_t;
// booster
typedef unsigned char boosterStatus_t;
// loco
typedef unsigned short locoID_t;
typedef unsigned char protocol_t;
typedef unsigned short address_t;
typedef unsigned short speed_t;
typedef bool direction_t;
typedef unsigned char function_t;
// layoutItem
typedef unsigned char layoutRotation_t;
typedef unsigned char layoutItemSize_t;
typedef unsigned char layoutPosition_t;
// accessory
typedef unsigned short accessoryID_t;
typedef unsigned char accessoryType_t;
typedef unsigned char accessoryState_t;
// feedback
typedef unsigned short feedbackID_t;
typedef unsigned int feedbackPin_t;
typedef unsigned char feedbackState_t;
// block
typedef unsigned short blockID_t;
typedef unsigned char blockState_t;

enum managerIDs : managerID_t {
	MANAGER_ID_CONSOLE = 0,
	MANAGER_ID_HARDWARE,
	MANAGER_ID_WEBSERVER
};

enum boosterStatus : boosterStatus_t {
	BOOSTER_STOP = 0,
	BOOSTER_GO
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

enum hardwareIDs : hardwareID_t {
	HARDWARE_ID_NONE = 0,
	HARDWARE_ID_VIRT,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

#define ADDRESS_NONE 0
#define LOCO_NONE 0;

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

enum feedbackState : feedbackState_t {
	FEEDBACK_STATE_FREE = 0,
	FEEDBACK_STATE_OCCUPIED
};

enum blockState : blockState_t {
	BLOCK_STATE_FREE = 0,
	BLOCK_STATE_RESERVED,
	BLOCK_STATE_USED
};

#endif // DATATYPES_H


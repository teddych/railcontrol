#pragma once

#include <string>

// common
typedef unsigned char controlType_t;
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
typedef unsigned char lockState_t;
// switch
typedef accessoryID_t switchID_t;
typedef accessoryType_t switchType_t;
typedef accessoryState_t switchState_t;
typedef accessoryTimeout_t switchTimeout_t;
// street
typedef objectID_t streetID_t;
typedef unsigned char lockState_t;

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
#define MIN_SPEED 0

#define WIDTH_1 1
#define HEIGHT_1 1

enum controlTypes : controlType_t {
	ControlTypeHardware = 0,
	ControlTypeAutomode,
	ControlTypeConsole,
	ControlTypeWebserver
};

enum controlIDs : controlID_t {
	ControlIdNone = 0,
	ControlIdConsole,
	ControlIdWebserver,
	ControlIdFirstHardware = 10
};

enum boosterStatus : boosterStatus_t {
	BoosterStop = 0,
	BoosterGo
};

enum protocols : protocol_t {
	ProtocolNone = 0,
	ProtocolServer,
	ProtocolMM1,
	ProtocolMM2,
	ProtocolMFX,
	ProtocolDCC,
	ProtocolDCCShort,
	ProtocolDCCLong,
	ProtocolSX1,
	ProtocolSX2,
	ProtocolEnd = ProtocolSX2
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
	AddressTypeLoco = 0,
	AddressTypeAccessory
};

enum hardwareTypes : hardwareType_t {
	HardwareTypeNone = 0,
	HardwareTypeVirt,
	HardwareTypeCS2,
	HardwareTypeNumbers
};

static std::string hardwareSymbols[] = {
	"none",
	"virtual",
	"cs2"
};

enum rotations : layoutRotation_t {
	Rotation0 = 0,
	Rotation90,
	Rotation180,
	Rotation270
};

typedef unsigned char objectType_t;

enum objectType : objectType_t {
	ObjectTypeLoco = 1,
	ObjectTypeBlock,
	ObjectTypeFeedback,
	ObjectTypeAccessory,
	ObjectTypeSwitch,
	ObjectTypeStreet
};

typedef unsigned char relationType_t;

enum relationType : relationType_t {
	RelationTypeBlockStreet = 0,
	RelationTypeStreetFeedback
};

enum accessoryColor : accessoryColor_t {
	AccessoryColorRed = 0,
	AccessoryColorGreen,
	AccessoryColorYellow,
	AccessoryColorWhite
};

enum accessoryType : accessoryType_t {
	AccessoryTypeDefault = 0
};

enum accessoryState : accessoryState_t {
	AccessoryStateOff = 0,
	AccessoryStateOn
};

enum feedbackState : feedbackState_t {
	FEEDBACK_STATE_FREE = 0,
	FEEDBACK_STATE_OCCUPIED
};

enum lockState : lockState_t {
	LOCK_STATE_FREE = 0,
	LOCK_STATE_RESERVED,
	LOCK_STATE_SOFT_LOCKED,
	LOCK_STATE_HARD_LOCKED
};

enum directionState : direction_t {
	DIRECTION_LEFT = false,
	DIRECTION_RIGHT = true
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


#pragma once

#include <string>

// common
typedef unsigned char controlID_t;

// objects in db
typedef unsigned short objectID_t;

// loco
typedef objectID_t locoID_t;
typedef unsigned short address_t;
typedef unsigned short LocoSpeed;
typedef unsigned char function_t;

// layoutItem
typedef unsigned char layoutItemSize_t;
typedef unsigned char layoutPosition_t;

// accessory
typedef objectID_t accessoryID_t;
typedef unsigned char accessoryType_t;
typedef bool accessoryState_t;
typedef unsigned short accessoryTimeout_t;

// feedback
typedef objectID_t feedbackID_t;
typedef unsigned int feedbackPin_t;

// block
typedef objectID_t blockID_t;

// switch
typedef accessoryID_t switchID_t;
typedef accessoryTimeout_t switchTimeout_t;
typedef accessoryState_t switchState_t;
typedef accessoryType_t switchType_t;

// street
typedef objectID_t streetID_t;

// relations in db
typedef unsigned short relationID_t;

static const address_t AddressNone = 0;
static const locoID_t LocoNone = 0;
static const accessoryID_t AccessoryNone = 0;
static const feedbackID_t FeedbackNone = 0;
static const blockID_t BlockNone = 0;
static const switchID_t SwitchNone = 0;
static const streetID_t StreetNone = 0;

static const LocoSpeed MaxSpeed = 1023;
static const LocoSpeed MinSpeed = 0;

static const layoutItemSize_t Width1 = 1;
static const layoutItemSize_t Height1 = 1;

enum controlType_t : unsigned char
{
	ControlTypeHardware = 0,
	ControlTypeAutomode,
	ControlTypeConsole,
	ControlTypeWebserver
};

enum controlIDs : controlID_t
{
	ControlIdNone = 0,
	ControlIdConsole,
	ControlIdWebserver,
	ControlIdFirstHardware = 10
};

enum boosterStatus_t : bool
{
	BoosterStop = false,
	BoosterGo = true
};

enum protocol_t : unsigned char
{
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

static const std::string protocolSymbols[] =
{
	"none",
	"all",
	"MM1",
	"MM2",
	"mfx",
	"DCC",
	"SX1",
	"SX2"
};

enum addressType_t : unsigned char
{
	AddressTypeLoco = 0,
	AddressTypeAccessory
};

enum hardwareType_t : unsigned char
{
	HardwareTypeNone = 0,
	HardwareTypeVirtual = 1,
	HardwareTypeCS2 = 2,
	HardwareTypeM6051 = 3,
	HardwareTypeNumbers
};

enum layoutRotation_t : unsigned char
{
	Rotation0 = 0,
	Rotation90,
	Rotation180,
	Rotation270
};

enum objectType_t : unsigned char
{
	ObjectTypeLoco = 1,
	ObjectTypeBlock,
	ObjectTypeFeedback,
	ObjectTypeAccessory,
	ObjectTypeSwitch,
	ObjectTypeStreet
};

enum relationType_t : unsigned char
{
	RelationTypeBlockStreet = 0,
	RelationTypeStreetFeedback
};

enum accessoryType : accessoryType_t
{
	AccessoryTypeDefault = 0
};

enum accessoryState : accessoryState_t
{
	AccessoryStateOff = false,
	AccessoryStateOn = true
};

enum feedbackState_t : unsigned char
{
	FeedbackStateFree = 0,
	FeedbackStateOccupied
};

enum lockState_t : unsigned char
{
	LockStateFree = 0,
	LockStateReserved,
	LockStateSoftLocked,
	LockStateHardLocked
};

enum direction_t : bool
{
	DirectionLeft = false,
	DirectionRight = true
};

enum switchType : switchType_t
{
	SwitchTypeLeft = 0,
	SwitchTypeRight
};

enum switchState : switchState_t
{
	SwitchStateTurnout = false,
	SwitchStateStraight = true
};

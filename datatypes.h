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

// track
typedef objectID_t trackID_t;
typedef unsigned char trackType_t;

// switch
typedef accessoryID_t switchID_t;
typedef accessoryTimeout_t switchTimeout_t;
typedef accessoryState_t switchState_t;
typedef accessoryType_t switchType_t;

// street
typedef objectID_t streetID_t;

// layer
typedef signed short layerID_t;
typedef unsigned char layer_t;

// relations
typedef unsigned short priority_t;

static const address_t AddressNone = 0;
static const locoID_t LocoNone = 0;
static const objectID_t ObjectNone = 0;
static const accessoryID_t AccessoryNone = 0;
static const feedbackID_t FeedbackNone = 0;
static const trackID_t TrackNone = 0;
static const switchID_t SwitchNone = 0;
static const streetID_t StreetNone = 0;
static const controlID_t ControlNone = 0;

static const LocoSpeed MaxSpeed = 1023;
static const LocoSpeed MinSpeed = 0;

static const layoutItemSize_t Width1 = 1;
static const layoutItemSize_t Height1 = 1;

enum controlType_t : unsigned char
{
	ControlTypeHardware = 0,
	ControlTypeInternal,
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

enum argumentType_t : unsigned char
{
	IpAddress = 1,
	SerialPort = 2,
	S88Modules = 3
};

enum hardwareType_t : unsigned char
{
	HardwareTypeNone = 0,
	HardwareTypeVirtual = 1,
	HardwareTypeCS2 = 2,
	HardwareTypeM6051 = 3,
	HardwareTypeRM485 = 4,
	HardwareTypeNumbers
};

enum layoutRotation_t : unsigned char
{
	Rotation0 = 0,
	Rotation90,
	Rotation180,
	Rotation270
};

enum visible_t : bool
{
	VisibleNo = false,
	VisibleYes = true
};

enum automode_t : bool
{
	AutomodeNo = false,
	AutomodeYes = true
};

enum objectType_t : unsigned char
{
	ObjectTypeLoco = 1,
	ObjectTypeTrack,
	ObjectTypeFeedback,
	ObjectTypeAccessory,
	ObjectTypeSwitch,
	ObjectTypeStreet,
	ObjectTypeLayer
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

enum feedbackState_t : bool
{
	FeedbackStateFree = false,
	FeedbackStateOccupied = true
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

enum trackType : trackType_t
{
	TrackTypeStraight = 0,
	TrackTypeLeft = 1,
	TrackTypeRight = 2
};

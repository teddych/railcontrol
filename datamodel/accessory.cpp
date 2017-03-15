#include "accessory.h"

namespace datamodel {

	Accessory::Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, accessoryType_t type, accessoryState_t state, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(ROTATION_0, /* width */ 1, /* height */ 1, x, y, z),
		accessoryID(accessoryID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		type(type),
		state(state) {
	}

	void Accessory::getAccessoryTexts(const accessoryState_t state, unsigned char& color, unsigned char& on, char*& colorText, char*& onText) {
		// calculate color as number
		color = state >> 1;
		// calculate color as text
		switch (color) {
			case 0:
				colorText = (char*)"red";
				break;
			case 1:
				colorText = (char*)"green";
				break;
			case 2:
				colorText = (char*)"yellow";
				break;
			case 3:
				colorText = (char*)"white";
				break;
			default:
				stateText = (char*)"unknown";
		}
		// calculate on as number
		on = state & 0x01;
		// calculate on as text
		switch (on) {
			case 0:
				onText = (char*)"off";
				break;
			case 1:
				onText = (char*)"on";
				break;
			default:
				stateText = (char*)"unknown";
		}
	}

} // namespace datamodel


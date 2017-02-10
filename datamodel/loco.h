#ifndef DATAMODEL_LOCO_H
#define DATAMODEL_LOCO_H

#include <string>

#include "../datatypes.h"

namespace datamodel {

class Loco {
	public:
		Loco(locoID_t locoID);
		Loco(locoID_t locoID, std::string name, protocol_t protocol, address_t address);
		~Loco();

		hardwareControlID_t hardwareControlID;
		locoID_t locoID;
		std::string name;
		protocol_t protocol;
		address_t address;
		speed_t speed;
};

} // namespace storage

#endif // DATAMODEL_LOCO_H

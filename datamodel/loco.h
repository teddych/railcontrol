#ifndef DATAMODEL_LOCO_H
#define DATAMODEL_LOCO_H

#include <string>

#include "datatypes.h"

namespace datamodel {

	class Loco {
		public:
			Loco(locoID_t locoID, std::string name, controlID_t controlID, protocol_t protocol, address_t address);

			locoID_t locoID;
			std::string name;
			controlID_t controlID;
			protocol_t protocol;
			address_t address;
			speed_t speed;
	};

} // namespace storage

#endif // DATAMODEL_LOCO_H

#ifndef DATAMODEL_ACCESSORY_H
#define DATAMODEL_ACCESSORY_H

#include <string>

#include "datatypes.h"

namespace datamodel {

	class Accessory {
		public:
			Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address);

			accessoryID_t accessoryID;
			std::string name;
			controlID_t controlID;
			protocol_t protocol;
			address_t address;
			accessoryState_t state;
	};

} // namespace storage

#endif // DATAMODEL_ACCESSORY_H

#ifndef DATAMODEL_LOCO_H
#define DATAMODEL_LOCO_H

#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Loco : Serializable {
		public:
			Loco(locoID_t locoID, std::string name, controlID_t controlID, protocol_t protocol, address_t address);
			Loco(const std::string);

			std::string serialize() const override;
			bool deserialize(std::string) override;

			locoID_t locoID;
			std::string name;
			controlID_t controlID;
			protocol_t protocol;
			address_t address;
			speed_t speed;
	};

} // namespace datamodel

#endif // DATAMODEL_LOCO_H

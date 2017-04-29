#ifndef DATAMODEL_LOCO_H
#define DATAMODEL_LOCO_H

#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel {

	class Loco : public Object {
		public:
			Loco(const locoID_t locoID, const std::string& name, const controlID_t controlID, const protocol_t protocol, const address_t address);
			Loco(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			controlID_t controlID;
			protocol_t protocol;
			address_t address;
			speed_t speed;
	};

} // namespace datamodel

#endif // DATAMODEL_LOCO_H

#ifndef DATAMODEL_STREET_H
#define DATAMODEL_STREET_H

#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel {

	class Street : public Object {
		public:
			Street(const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const direction_t fromDirection, const blockID_t toBlock, const direction_t toDirection);
			Street(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			bool fromBlockDirection(blockID_t blockID, direction_t direction);

		private:
			blockID_t fromBlock;
			direction_t fromDirection;
			blockID_t toBlock;
			direction_t toDirection;
			locoID_t locoID;
	};

} // namespace datamodel

#endif // DATAMODEL_STREET_H

#ifndef DATAMODEL_STREET_H
#define DATAMODEL_STREET_H

#include <string>

#include "datatypes.h"
#include "relation.h"

namespace datamodel {

	class Street : private Relation {
		public:
			Street(streetID_t streetID, std::string name);
			Street(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			streetID_t streetID;
			std::string name;
	};

} // namespace datamodel

#endif // DATAMODEL_STREET_H

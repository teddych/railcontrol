#ifndef DATAMODEL_STREET_H
#define DATAMODEL_STREET_H

#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel {

	class Street : public Object {
		public:
			Street(const streetID_t streetID, const std::string& name);
			Street(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;
	};

} // namespace datamodel

#endif // DATAMODEL_STREET_H

#ifndef DATAMODEL_STREET_H
#define DATAMODEL_STREET_H

#include <string>

#include "datatypes.h"
#include "object.h"

namespace datamodel {

	class Street : public Object {
		public:
			Street(const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const blockID_t toBlock);
			Street(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

		private:
			blockID_t fromBlock;
			blockID_t toBlock;
	};

} // namespace datamodel

#endif // DATAMODEL_STREET_H

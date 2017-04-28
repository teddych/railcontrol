#ifndef DATAMODEL_RELATION_H
#define DATAMODEL_RELATION_H

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Relation : protected Serializable {
		public:
			Relation() {};

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);

			relationType_t relationType;
			objectID_t objectID1;
			objectID_t objectID2;
	};

} // namespace datamodel

#endif // DATAMODEL_RELATION_H

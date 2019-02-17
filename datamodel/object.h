#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Object : protected Serializable {
		public:
			Object() {}
			Object(const objectID_t objectID, const std::string& name)
			:	objectID(objectID),
				name(name)
			{
			}

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;

			const std::string& Name() const { return name; }

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);

		// FIXME: make private
		public:
			objectID_t objectID;
			std::string name;
	};

} // namespace datamodel


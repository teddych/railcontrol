#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel
{
	class Object : protected Serializable
	{
		public:
			Object() {}
			Object(const objectID_t objectID, const std::string& name)
			:	objectID(objectID),
				name(name)
			{
			}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;

			void SetID(const objectID_t id) { this->objectID = id; }
			objectID_t GetID() const { return objectID; }
			void SetName(const std::string& name) { this->name = name; }
			const std::string& GetName() const { return name; }

		protected:
			virtual bool Deserialize(const std::map<std::string,std::string>& arguments);

			objectID_t objectID;
			std::string name;
	};
} // namespace datamodel


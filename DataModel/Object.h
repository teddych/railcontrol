#pragma once

#include <map>
#include <string>

#include "DataTypes.h"
#include "DataModel/Serializable.h"

namespace DataModel
{
	class Object : protected Serializable
	{
		public:
			Object() {}
			Object(const objectID_t objectID) : objectID(objectID) {}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;

			objectID_t GetID() const { return objectID; }
			virtual void SetName(const std::string& name) { this->name = name; }
			const std::string& GetName() const { return name; }

		protected:
			virtual bool Deserialize(const std::map<std::string,std::string>& arguments);

			objectID_t objectID;
			std::string name;
	};
} // namespace DataModel


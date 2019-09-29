#pragma once

#include <map>
#include <string>

#include "DataTypes.h"
#include "DataModel/Serializable.h"

class Manager;

namespace DataModel
{
	class Layer : public Object
	{
		public:
			Layer(const std::string& serialized) { Deserialize(serialized); }
			Layer(__attribute__((unused)) Manager* manager, const layerID_t layerID) : Object(layerID) {}

			objectType_t GetObjectType() const { return ObjectTypeLayer; }
	};
} // namespace DataModel


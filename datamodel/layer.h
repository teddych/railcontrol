#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel
{
	class Layer : public Object
	{
		public:
			Layer(const std::string& serialized) { deserialize(serialized); }
			Layer(const layerID_t layerID, const std::string& name)
			:	Object(layerID, name)
			{
			}
	};
} // namespace datamodel


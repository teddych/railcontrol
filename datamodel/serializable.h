#pragma once

#include <map>
#include <string>
#include <vector>

#include "util.h"

namespace datamodel {

	class Serializable {
		public:
			virtual ~Serializable() {};
			virtual std::string Serialize() const = 0;
			virtual bool Deserialize(const std::string& serialized) = 0;

		protected:
			void ParseArguments(std::string serialized, std::map<std::string,std::string>& arguments);
	};

} // namespace datamodel


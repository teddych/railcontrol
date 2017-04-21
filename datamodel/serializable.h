#ifndef DATAMODEL_SERIALIZABLE_H
#define DATAMODEL_SERIALIZABLE_H

#include <map>
#include <string>
#include <vector>

#include "util.h"

namespace datamodel {

	class Serializable {
		public:
			virtual ~Serializable() {};
			virtual std::string serialize() const = 0;
			virtual bool deserialize(const std::string) = 0;

		protected:
			void parseArguments(std::string serialized, std::map<std::string,std::string>& arguments);
	};

} // namespace datamodel

#endif // DATAMODEL_SERIALIZABLE_H

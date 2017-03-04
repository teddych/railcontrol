#ifndef DATAMODEL_BLOCK_H
#define DATAMODEL_BLOCK_H

#include <mutex>
#include <string>

#include "datatypes.h"

namespace datamodel {

	class Block {
		public:
			Block(blockID_t blockID, std::string name);

			bool tryReserve(const locoID_t locoID);
			bool reserve(const locoID_t locoID);
			bool free(const locoID_t locoID);

			blockID_t blockID;
			std::string name;
		private:
			blockState_t state;
			locoID_t locoID;
			std::mutex updateMutex;
	};

} // namespace datamodel

#endif // DATAMODEL_BLOCK_H

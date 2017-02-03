#ifndef STORAGE_STORAGE_H
#define STORAGE_STORAGE_H

#include <vector>

#include "../datamodel/datamodel.h"

namespace storage {

class Storage {
	public:
		Storage();
		~Storage();
		void loco(const datamodel::Loco& loco);
		std::vector<datamodel::Loco*> allLocos();
};

} // namespace storage

#endif // STORAGE_STORAGE_H

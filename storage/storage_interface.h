#ifndef STORAGE_STORAGE_INTERFACE_H
#define STORAGE_STORAGE_INTERFACE_H

#include <map>
#include <string>

#include "../datamodel/datamodel.h"
#include "../datatypes.h"

namespace storage {

	class StorageInterface {
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			StorageInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~StorageInterface() {};

			// save loco
			virtual void loco(const datamodel::Loco& loco) = 0;

			// read all locos
			virtual void allLocos(std::map<locoID_t, datamodel::Loco*>& locos) = 0;
	};

} // namespace


#endif // STORAGE_STORAGE_INTERFACE_H


#ifndef STORAGE_STORAGE_INTERFACE_H
#define STORAGE_STORAGE_INTERFACE_H

#include <string>

#include "../datatypes.h"

namespace storage {

	class StorageInterface {
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			StorageInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~StorageInterface() {};

			// save loco
			void loco(const datamodel::Loco& loco);

			// read all locos
			std::vector<datamodel::Loco*> allLocos();
	};

} // namespace


#endif // STORAGE_STORAGE_INTERFACE_H


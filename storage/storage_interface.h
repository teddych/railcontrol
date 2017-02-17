#ifndef STORAGE_STORAGE_INTERFACE_H
#define STORAGE_STORAGE_INTERFACE_H

#include <map>
#include <string>

#include "datamodel/datamodel.h"
#include "datatypes.h"
#include "hardware/hardware_params.h"

namespace storage {

	class StorageInterface {
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			StorageInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~StorageInterface() {};

			// save control
			virtual void hardwareParams(const hardware::HardwareParams& hardwareParams) = 0;

			// read controls
			virtual void allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams) = 0;
			
			// save loco
			virtual void loco(const datamodel::Loco& loco) = 0;

			// read all locos
			virtual void allLocos(std::map<locoID_t, datamodel::Loco*>& locos) = 0;
	};

} // namespace


#endif // STORAGE_STORAGE_INTERFACE_H


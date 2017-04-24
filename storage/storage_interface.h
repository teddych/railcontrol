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

			// save datamodelobject
			virtual void saveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object) = 0;

			// read datamodelobject
			virtual void objectsOfType(const objectType_t objectType, std::vector<std::string>& objects) = 0;
	};

} // namespace


#endif // STORAGE_STORAGE_INTERFACE_H


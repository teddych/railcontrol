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

			// save accessory
			virtual void accessory(const datamodel::Accessory& accessory) = 0;

			// read all accessories
			virtual void allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories) = 0;

			// save feedback
			virtual void feedback(const datamodel::Feedback& feedback) = 0;

			// read all feedbacks
			virtual void allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks) = 0;

			// save block
			virtual void block(const datamodel::Block& block) = 0;

			// read all blocks
			virtual void allBlocks(std::map<blockID_t,datamodel::Block*>& blocks) = 0;
	};

} // namespace


#endif // STORAGE_STORAGE_INTERFACE_H


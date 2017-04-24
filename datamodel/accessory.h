#ifndef DATAMODEL_ACCESSORY_H
#define DATAMODEL_ACCESSORY_H

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

namespace datamodel {

	class Accessory : public LayoutItem {
		public:
			Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, accessoryType_t type, accessoryState_t state, accessoryTimeout_t timeout, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z);
			Accessory(const std::string& serialized);
			Accessory();

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;

			accessoryID_t accessoryID;
			std::string name;
			controlID_t controlID;
			protocol_t protocol;
			address_t address;
			accessoryType_t type;
			accessoryState_t state; // first 7 bits are for state/color, last bit is for on/off
			accessoryTimeout_t timeout; // timeout in ms after which the accessory command will be turned off on rails. 0 = no turn off / turn off must be made manually

			static void getAccessoryTexts(const accessoryState_t state, unsigned char& color, unsigned char& on, char*& colorText, char*& onText);

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);
	};

	inline Accessory::Accessory() {}

} // namespace datamodel

#endif // DATAMODEL_ACCESSORY_H

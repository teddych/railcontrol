#pragma once

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"

namespace datamodel {

	class Accessory : public LayoutItem {
		public:
			Accessory(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryState_t state, const accessoryTimeout_t timeout, const bool inverted);
			Accessory(const std::string& serialized);
			Accessory();

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;
			virtual std::string layoutType() const override { return "accessory"; };

			virtual void Inverted(const bool inverted) { this->inverted = inverted; }
			virtual bool IsInverted() const { return inverted; }

			// FIXME: make this private
			controlID_t controlID;
			protocol_t protocol;
			address_t address;
			accessoryType_t type;
			accessoryState_t state;
			accessoryTimeout_t timeout; // timeout in ms after which the accessory command will be turned off on rails. 0 = no turn off / turn off must be made manually

		protected:
			std::string serializeWithoutType() const;
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);

			bool inverted;
	};

	inline Accessory::Accessory() {}

} // namespace datamodel


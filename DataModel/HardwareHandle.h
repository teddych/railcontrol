#pragma once

#include <map>
#include <sstream>

#include "datatypes.h"
#include "DataModel/Serializable.h"

namespace DataModel
{
	class HardwareHandle : protected Serializable
	{
		public:
			HardwareHandle(const controlID_t controlID,
				const protocol_t protocol,
				const address_t address)
			:	controlID(controlID),
				protocol(protocol),
				address(address)
			{
			}

			HardwareHandle(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			HardwareHandle() {}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;

			void SetControlID(controlID_t controlID) { this->controlID = controlID; }
			controlID_t GetControlID() const { return controlID; }
			void SetProtocol(protocol_t protocol) { this->protocol = protocol; }
			protocol_t GetProtocol() const { return protocol; }
			void SetAddress(address_t address) { this->address = address; }
			address_t GetAddress() const { return address; }

		protected:
			virtual bool Deserialize(const std::map<std::string,std::string>& arguments);

		private:
			controlID_t controlID;
			protocol_t protocol;
			address_t address;
	};
} // namespace DataModel


#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "hardware/HardwareParams.h"

namespace storage
{
	class StorageInterface
	{
		public:
		  // non virtual default constructor is needed to prevent polymorphism
			StorageInterface() {};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~StorageInterface() {};

			// save control
			virtual void SaveHardwareParams(const hardware::HardwareParams& hardwareParams) = 0;

			// read controls
			virtual void AllHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams) = 0;

			// delete control
			virtual void DeleteHardwareParams(const controlID_t controlID) = 0;

			// save datamodelobject
			virtual void SaveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object) = 0;

			// delete datamodelobject
			virtual void DeleteObject(const objectType_t objectType, const objectID_t objectID) = 0;

			// read datamodelobject
			virtual void ObjectsOfType(const objectType_t objectType, std::vector<std::string>& objects) = 0;

			// save datamodelrelation
			virtual void SaveRelation(const objectType_t objectType1, const objectID_t objectID1, const objectType_t objectType2, const objectID_t objectID2, const priority_t priority, const std::string& relation) = 0;

			// delete datamodelrelation
			virtual void DeleteRelationFrom(const objectType_t objectType, const objectID_t objectID) = 0;

			// delete datamodelrelation
			virtual void DeleteRelationTo(const objectType_t objectType, const objectID_t objectID) = 0;

			// read datamodelrelation
			virtual void RelationsFrom(const objectType_t objectType, const objectID_t objectID, std::vector<std::string>& relations) = 0;

			// read datamodelrelation
			virtual void RelationsTo(const objectType_t objectType, const objectID_t objectID, std::vector<std::string>& relations) = 0;

			// save setting
			virtual void SaveSetting(const std::string& key, const std::string& value) = 0;

			// read setting
			virtual std::string GetSetting(const std::string& key) = 0;
	};
} // namespace


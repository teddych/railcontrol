#ifndef AMALGAMATION
#include <dlfcn.h>              // dl*
#endif
#include <sstream>
#include <string>
#include <vector>

#include "Logger/Logger.h"
#include "storage/StorageHandler.h"
#include "util.h"

using datamodel::Accessory;
using datamodel::Track;
using datamodel::Feedback;
using datamodel::Layer;
using datamodel::Loco;
using datamodel::Relation;
using datamodel::Street;
using datamodel::Switch;
using std::map;
using std::string;
using std::vector;

namespace storage {

	StorageHandler::StorageHandler(Manager* manager, const StorageParams& params)
	:	manager(manager),
		createStorage(nullptr),
		destroyStorage(nullptr),
		instance(nullptr),
		dlhandle(nullptr)
	{
#ifdef AMALGAMATION
		createStorage = (storage::StorageInterface* (*)(storage::StorageParams))(&create_sqlite);
		destroyStorage = (void (*)(storage::StorageInterface*))(&destroy_sqlite);
#else
		// generate symbol and library names
		char* error;
		std::stringstream ss;
		ss << "storage/" << params.module << ".so";

		Logger::Logger* logger = Logger::Logger::GetLogger("StorageHandler");
		dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
		if (!dlhandle)
		{
			logger->Error("Can not open storage library: {0}", dlerror());
			return;
		}

		// look for symbol create_*
		ss.str(std::string());
		ss << "create_" << params.module;
		string s = ss.str();
		createStorage_t* newCreateStorage = (createStorage_t*) dlsym(dlhandle, s.c_str());
		error = dlerror();
		if (error)
		{
			logger->Error("Unable to find symbol {0}", s);
			return;
		}

		// look for symbol destroy_*
		ss.str(std::string());
		ss << "destroy_" << params.module;
		s = ss.str();
		destroyStorage_t* newDestroyStorage = (destroyStorage_t*) dlsym(dlhandle, s.c_str());
		error = dlerror();
		if (error)
		{
			logger->Error("Unable to find symbol {0}", s);
			return;
		}

		// register  valid symbols
		createStorage = newCreateStorage;
		destroyStorage = newDestroyStorage;
#endif

		// start storage
		if (createStorage)
		{
			instance = createStorage(params);
		}
	}

	StorageHandler::~StorageHandler()
	{
		// stop storage
		if (instance)
		{
			destroyStorage(instance);
			instance = NULL;
		}

#ifndef AMALGAMATION
		// close library
		if (dlhandle)
		{
			dlclose(dlhandle);
			dlhandle = NULL;
		}
#endif
	}

	void StorageHandler::Save(const hardware::HardwareParams& hardwareParams)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->SaveHardwareParams(hardwareParams);
	}

	void StorageHandler::AllHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->AllHardwareParams(hardwareParams);
	}

	void StorageHandler::DeleteHardwareParams(const controlID_t controlID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteHardwareParams(controlID);
	}

	void StorageHandler::Save(const Loco& loco)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = loco.Serialize();
		instance->SaveObject(ObjectTypeLoco, loco.objectID, loco.name, serialized);
	}

	void StorageHandler::AllLocos(map<locoID_t,datamodel::Loco*>& locos)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeLoco, objects);
		for(auto object : objects) {
			Loco* loco = new Loco(manager, object);
			locos[loco->objectID] = loco;
		}
	}

	void StorageHandler::DeleteLoco(const locoID_t locoID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeLoco, locoID);
	}

	void StorageHandler::Save(const Accessory& accessory)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = accessory.Serialize();
		instance->SaveObject(ObjectTypeAccessory, accessory.objectID, accessory.name, serialized);
	}

	void StorageHandler::AllAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeAccessory, objects);
		for(auto object : objects)
		{
			Accessory* accessory = new Accessory(object);
			accessories[accessory->objectID] = accessory;
		}
	}

	void StorageHandler::DeleteAccessory(const accessoryID_t accessoryID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeAccessory, accessoryID);
	}

	void StorageHandler::Save(const Feedback& feedback)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = feedback.Serialize();
		instance->SaveObject(ObjectTypeFeedback, feedback.objectID, feedback.name, serialized);
	}

	void StorageHandler::AllFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeFeedback, objects);
		for(auto object : objects)
		{
			Feedback* feedback = new Feedback(manager, object);
			feedbacks[feedback->objectID] = feedback;
		}
	}

	void StorageHandler::DeleteFeedback(const feedbackID_t feedbackID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeFeedback, feedbackID);
	}

	void StorageHandler::Save(const Track& track)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = track.Serialize();
		instance->SaveObject(ObjectTypeTrack, track.objectID, track.name, serialized);
	}

	void StorageHandler::AllTracks(std::map<trackID_t,datamodel::Track*>& tracks)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeTrack, objects);
		for(auto object : objects)
		{
			Track* track = new Track(manager, object);
			tracks[track->objectID] = track;
		}
	}

	void StorageHandler::DeleteTrack(const trackID_t trackID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeTrack, trackID);
	}

	void StorageHandler::Save(const Switch& mySwitch)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = mySwitch.Serialize();
		instance->SaveObject(ObjectTypeSwitch, mySwitch.objectID, mySwitch.name, serialized);
	}

	void StorageHandler::AllSwitches(std::map<switchID_t,datamodel::Switch*>& switches)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeSwitch, objects);
		for(auto object : objects)
		{
			Switch* mySwitch = new Switch(object);
			switches[mySwitch->objectID] = mySwitch;
		}
	}

	void StorageHandler::DeleteSwitch(const switchID_t switchID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeSwitch, switchID);
	}

	void StorageHandler::Save(const datamodel::Street& street)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = street.Serialize();
		instance->SaveObject(ObjectTypeStreet, street.objectID, street.name, serialized);
		instance->DeleteRelationFrom(ObjectTypeStreet, street.objectID);
		const vector<datamodel::Relation*> relations = street.GetRelations();
		for (auto relation : relations)
		{
			string serializedRelation = relation->Serialize();
			instance->SaveRelation(ObjectTypeStreet, street.objectID, relation->ObjectType2(), relation->ObjectID2(), relation->Priority(), serializedRelation);
		}
	}

	void StorageHandler::AllStreets(std::map<streetID_t,datamodel::Street*>& streets)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeStreet, objects);
		for(auto object : objects) {
			Street* street = new Street(manager, object);
			vector<string> relationsString;
			instance->RelationsFrom(ObjectTypeStreet, street->objectID, relationsString);
			vector<Relation*> relations;
			for (auto relationString : relationsString)
			{
				relations.push_back(new Relation(manager, relationString));
			}
			street->AssignRelations(relations);
			streets[street->objectID] = street;
		}
	}

	void StorageHandler::DeleteStreet(const streetID_t streetID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeStreet, streetID);
	}

	void StorageHandler::Save(const datamodel::Layer& layer)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = layer.Serialize();
		instance->SaveObject(ObjectTypeLayer, layer.objectID, layer.name, serialized);
	}

	void StorageHandler::AllLayers(std::map<layerID_t,datamodel::Layer*>& layers)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->ObjectsOfType(ObjectTypeLayer, objects);
		for(auto object : objects) {
			Layer* layer = new Layer(object);
			layers[layer->objectID] = layer;
		}
	}

	void StorageHandler::DeleteLayer(const layerID_t layerID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->DeleteObject(ObjectTypeLayer, layerID);
	}

	void StorageHandler::SaveSetting(const std::string& key, const std::string& value)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->SaveSetting(key, value);
	}

	std::string StorageHandler::GetSetting(const std::string& key)
	{
		if (instance == nullptr)
		{
			return "";
		}
		return instance->GetSetting(key);
	}
} // namespace storage


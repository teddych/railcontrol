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

	void StorageHandler::hardwareParams(const hardware::HardwareParams& hardwareParams)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->hardwareParams(hardwareParams);
	}

	void StorageHandler::allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->allHardwareParams(hardwareParams);
	}

	void StorageHandler::deleteHardwareParams(const controlID_t controlID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteHardwareParams(controlID);
	}

	void StorageHandler::loco(const Loco& loco)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = loco.Serialize();
		instance->saveObject(ObjectTypeLoco, loco.objectID, loco.name, serialized);
	}

	void StorageHandler::allLocos(map<locoID_t,datamodel::Loco*>& locos)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeLoco, objects);
		for(auto object : objects) {
			Loco* loco = new Loco(manager, object);
			locos[loco->objectID] = loco;
		}
	}

	void StorageHandler::deleteLoco(const locoID_t locoID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeLoco, locoID);
	}

	void StorageHandler::accessory(const Accessory& accessory)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = accessory.Serialize();
		instance->saveObject(ObjectTypeAccessory, accessory.objectID, accessory.name, serialized);
	}

	void StorageHandler::allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeAccessory, objects);
		for(auto object : objects)
		{
			Accessory* accessory = new Accessory(object);
			accessories[accessory->objectID] = accessory;
		}
	}

	void StorageHandler::deleteAccessory(const accessoryID_t accessoryID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeAccessory, accessoryID);
	}

	void StorageHandler::feedback(const Feedback& feedback)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = feedback.Serialize();
		instance->saveObject(ObjectTypeFeedback, feedback.objectID, feedback.name, serialized);
	}

	void StorageHandler::allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeFeedback, objects);
		for(auto object : objects)
		{
			Feedback* feedback = new Feedback(manager, object);
			feedbacks[feedback->objectID] = feedback;
		}
	}

	void StorageHandler::deleteFeedback(const feedbackID_t feedbackID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeFeedback, feedbackID);
	}

	void StorageHandler::track(const Track& track)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = track.Serialize();
		instance->saveObject(ObjectTypeTrack, track.objectID, track.name, serialized);
	}

	void StorageHandler::allTracks(std::map<trackID_t,datamodel::Track*>& tracks)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeTrack, objects);
		for(auto object : objects)
		{
			Track* track = new Track(object);
			tracks[track->objectID] = track;
		}
	}

	void StorageHandler::deleteTrack(const trackID_t trackID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeTrack, trackID);
	}

	void StorageHandler::saveSwitch(const Switch& mySwitch)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = mySwitch.Serialize();
		instance->saveObject(ObjectTypeSwitch, mySwitch.objectID, mySwitch.name, serialized);
	}

	void StorageHandler::allSwitches(std::map<switchID_t,datamodel::Switch*>& switches)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeSwitch, objects);
		for(auto object : objects)
		{
			Switch* mySwitch = new Switch(object);
			switches[mySwitch->objectID] = mySwitch;
		}
	}

	void StorageHandler::deleteSwitch(const switchID_t switchID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeSwitch, switchID);
	}

	void StorageHandler::street(const datamodel::Street& street)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = street.Serialize();
		instance->saveObject(ObjectTypeStreet, street.objectID, street.name, serialized);
		instance->deleteRelationFrom(ObjectTypeStreet, street.objectID);
		const vector<datamodel::Relation*> relations = street.GetRelations();
		for (auto relation : relations)
		{
			string serializedRelation = relation->Serialize();
			instance->saveRelation(ObjectTypeStreet, street.objectID, relation->ObjectType2(), relation->ObjectID2(), relation->Priority(), serializedRelation);
		}
	}

	void StorageHandler::allStreets(std::map<streetID_t,datamodel::Street*>& streets)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeStreet, objects);
		for(auto object : objects) {
			Street* street = new Street(manager, object);
			vector<string> relationsString;
			instance->relationsFrom(ObjectTypeStreet, street->objectID, relationsString);
			vector<Relation*> relations;
			for (auto relationString : relationsString)
			{
				relations.push_back(new Relation(relationString));
			}
			street->AssignRelations(relations);
			streets[street->objectID] = street;
		}
	}

	void StorageHandler::deleteStreet(const streetID_t streetID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeStreet, streetID);
	}

	void StorageHandler::layer(const datamodel::Layer& layer)
	{
		if (instance == nullptr)
		{
			return;
		}
		string serialized = layer.Serialize();
		instance->saveObject(ObjectTypeLayer, layer.objectID, layer.name, serialized);
	}

	void StorageHandler::allLayers(std::map<layerID_t,datamodel::Layer*>& layers)
	{
		if (instance == nullptr)
		{
			return;
		}
		vector<string> objects;
		instance->objectsOfType(ObjectTypeLayer, objects);
		for(auto object : objects) {
			Layer* layer = new Layer(object);
			layers[layer->objectID] = layer;
		}
	}

	void StorageHandler::deleteLayer(const layerID_t layerID)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->deleteObject(ObjectTypeLayer, layerID);
	}
} // namespace storage


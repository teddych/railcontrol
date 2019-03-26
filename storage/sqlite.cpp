#include <map>
#include <sstream>
#include <fstream>
#include <string>

#include "storage/sqlite.h"

using datamodel::Accessory;
using datamodel::Track;
using datamodel::Feedback;
using datamodel::Loco;
using datamodel::Switch;
using hardware::HardwareParams;
using std::map;
using std::string;
using std::stringstream;
using std::vector;

namespace storage
{

	// create instance of sqlite
	extern "C" SQLite* create_sqlite(const StorageParams& params)
	{
		return new SQLite(params);
	}

	// delete instance of sqlite
	extern "C" void destroy_sqlite(SQLite* sqlite)
	{
		delete (sqlite);
	}

	SQLite::SQLite(const StorageParams& params)
	:	filename(params.filename),
	 	logger(Logger::Logger::GetLogger("SQLite"))
	{
		int rc;
		char* dbError = nullptr;

		logger->Info("Loading SQLite database with filename {0}", filename);
		rc = sqlite3_open(filename.c_str(), &db);
		if (rc)
		{
			logger->Error("Unable to load SQLite database: {0}", sqlite3_errmsg(db));
			sqlite3_close(db);
			db = nullptr;
			return;
		}

		// check if needed tables exist
		map<string, bool> tablenames;
		rc = sqlite3_exec(db, "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;", CallbackListTables, &tablenames, &dbError);
		if (rc != SQLITE_OK)
		{
			logger->Error("SQLite error: {0}", dbError);
			sqlite3_free(dbError);
			sqlite3_close(db);
			db = nullptr;
			return;
		}

		// create hardware table if needed
		if (tablenames["hardware"] != true)
		{
			logger->Info("Creating table hardware");
			rc = sqlite3_exec(db, "CREATE TABLE hardware ("
				"controlid UNSIGNED TINYINT PRIMARY KEY,"
				" hardwaretype UNSIGNED TINYINT,"
				" name VARCHAR(50),"
				" arg1 VARCHAR(255),"
				" arg2 VARCHAR(255),"
				" arg3 VARCHAR(255),"
				" arg4 VARCHAR(255),"
				" arg5 VARCHAR(255));", nullptr, nullptr, &dbError);
			if (rc != SQLITE_OK)
			{
				logger->Error("SQLite error: {0}", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = nullptr;
				return;
			}
		}

		// create objects table if needed
		if (tablenames["objects"] != true)
		{
			logger->Info("Creating table objects");
			rc = sqlite3_exec(db, "CREATE TABLE objects ("
				"objecttype UNSIGNED TINYINT, "
				"objectid UNSIGNED SHORTINT, "
				"name VARCHAR(50), "
				"object SHORTTEXT,"
				"PRIMARY KEY (objecttype, objectid));",
			nullptr, nullptr, &dbError);
			if (rc != SQLITE_OK)
			{
				logger->Error("SQLite error: {0}", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = nullptr;
				return;
			}
		}

		// create relations table if needed
		if (tablenames["relations"] != true)
		{
			logger->Info("Creating table relations");
			rc = sqlite3_exec(db, "CREATE TABLE relations ("
				"objecttype1 UNSIGNED TINYINT, "
				"objectid1 UNSIGNED SHORTINT, "
				"objecttype2 UNSIGNED TINYINT, "
				"objectid2 UNSIGNED SHORTINT, "
				"priority UNSIGNED TINYINT, "
				"relation SHORTTEXT,"
				"PRIMARY KEY (objecttype1, objectid1, objecttype2, objectid2, priority));",
			nullptr, nullptr, &dbError);
			if (rc != SQLITE_OK)
			{
				logger->Error("SQLite error: {0}", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = nullptr;
				return;
			}
		}

		// create settings table if needed
		if (tablenames["settings"] != true)
		{
			logger->Info("Creating table settings");
			rc = sqlite3_exec(db, "CREATE TABLE settings ("
				"key TINYTEXT, "
				"value SHORTTEXT,"
				"PRIMARY KEY (key));",
			nullptr, nullptr, &dbError);
			if (rc != SQLITE_OK)
			{
				logger->Error("SQLite error: {0}", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = nullptr;
				return;
			}
		}
	}

	SQLite::~SQLite()
	{
		if (db == nullptr)
		{
			return;
		}

		logger->Info("Closing SQLite database");
		sqlite3_close(db);
		db = nullptr;

		string sourceFilename(filename);
		string destinationFilename(filename + "." + std::to_string(time(0)));
		logger->Info("Copying from {0} to {1}", sourceFilename, destinationFilename);
		std::ifstream source(sourceFilename, std::ios::binary);
		std::ofstream destination(destinationFilename, std::ios::binary);
		destination << source.rdbuf();
		source.close();
		destination.close();
	}

	int SQLite::CallbackListTables(void* v, int argc, char **argv, char **colName)
	{
		map<string, bool>* tablenames = static_cast<map<string, bool>*>(v);
		(*tablenames)[argv[0]] = true;
		return 0;
	}

	void SQLite::SaveHardwareParams(const hardware::HardwareParams& hardwareParams)
	{
		stringstream ss;
		ss << "INSERT OR REPLACE INTO hardware VALUES ("
			<< (int) hardwareParams.controlID << ", "
			<< (int) hardwareParams.hardwareType << ", '"
			<< hardwareParams.name << "', '"
			<< hardwareParams.arg1 << "', '"
			<< hardwareParams.arg2 << "', '"
			<< hardwareParams.arg3 << "', '"
			<< hardwareParams.arg4 << "', '"
			<< hardwareParams.arg5
			<< "');";
		Execute(ss.str());
	}

	void SQLite::AllHardwareParams(std::map<controlID_t, hardware::HardwareParams*>& hardwareParams)
	{
		if (db == nullptr)
		{
			return;
		}

		char* dbError = 0;
		int rc = sqlite3_exec(db, "SELECT controlid, hardwaretype, name, arg1, arg2, arg3, arg4, arg5 FROM hardware ORDER BY controlid;", CallbackAllHardwareParams, &hardwareParams, &dbError);
		if (rc == SQLITE_OK)
		{
			return;
		}

		logger->Error("SQLite error: {0}", dbError);
		sqlite3_free(dbError);
	}

	// callback read hardwareparams
	int SQLite::CallbackAllHardwareParams(void* v, int argc, char **argv, char **colName)
	{
		map<controlID_t,HardwareParams*>* hardwareParams = static_cast<map<controlID_t,HardwareParams*>*>(v);
		if (argc != 8)
		{
			return 0;
		}
		controlID_t controlID = atoi(argv[0]);

		HardwareParams* params = new HardwareParams(controlID, static_cast<hardwareType_t>(atoi(argv[1])), argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
		(*hardwareParams)[controlID] = params;
		return 0;
	}

	// delete control
	void SQLite::DeleteHardwareParams(const controlID_t controlID)
	{
		stringstream ss;
		ss << "DELETE FROM hardware WHERE controlid = " << (int)controlID << ";";
		Execute(ss.str());
	}

	// save datamodelobject
	void SQLite::SaveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object)
	{
		stringstream ss;
		// FIXME: escape "'" in object
		ss << "INSERT OR REPLACE INTO objects (objecttype, objectid, name, object) VALUES (" << (int)objectType << ", " << (int)objectID << ", '" << name << "', '" << object << "');";
		Execute(ss.str());
	}

	// delete datamodelobject
	void SQLite::DeleteObject(const objectType_t objectType, const objectID_t objectID)
	{
		stringstream ss;
		ss << "DELETE FROM objects WHERE objecttype = " << (int) objectType << " AND objectid = " << (int) objectID << ";";
		Execute(ss.str());
	}

	// read datamodelobjects
	void SQLite::ObjectsOfType(const objectType_t objectType, vector<string>& objects)
	{
		if (db == nullptr)
		{
			return;
		}

		char* dbError = 0;
		stringstream ss;
		ss << "SELECT object FROM objects WHERE objecttype = " << (int) objectType << " ORDER BY objectid;";
		string s(ss.str());
		int rc = sqlite3_exec(db, s.c_str(), CallbackStringVector, &objects, &dbError);
		if (rc == SQLITE_OK)
		{
			return;
		}

		logger->Error("SQLite error: {0} Query: {1}", dbError, s);
		sqlite3_free(dbError);
	}

	// save datamodelrelation
	void SQLite::SaveRelation(const objectType_t objectType1, const objectID_t objectID1, const objectType_t objectType2, const objectID_t objectID2, const priority_t priority, const std::string& relation)
	{
		stringstream ss;
		// FIXME: escape "'" in relation
		ss << "INSERT OR REPLACE INTO relations (objecttype1, objectid1, objecttype2, objectid2, priority, relation) VALUES ("
			<< static_cast<int>(objectType1) << ", "
			<< static_cast<int>(objectID1) << ", "
			<< static_cast<int>(objectType2) << ", "
			<< static_cast<int>(objectID2) << ", "
			<< static_cast<int>(priority) << ", '"
			<< relation << "');";
		Execute(ss.str());
	}

	// delete datamodelrelaton
	void SQLite::DeleteRelationFrom(const objectType_t objectType, const objectID_t objectID)
	{
		stringstream ss;
		ss << "DELETE FROM relations WHERE objecttype1 = " << (int) objectType << " AND objectid1 = " << (int) objectID << ";";
		Execute(ss.str());
	}

	// delete datamodelrelaton
	void SQLite::DeleteRelationTo(const objectType_t objectType, const objectID_t objectID)
	{
		stringstream ss;
		ss << "DELETE FROM relations WHERE objecttype2 = " << (int) objectType << " AND objectid2 = " << (int) objectID << ";";
		Execute(ss.str());
	}

	// read datamodelrelations
	void SQLite::RelationsFrom(const objectType_t objectType, const objectID_t objectID, vector<string>& relations)
	{
		if (db == nullptr)
		{
			return;
		}

		char* dbError = 0;
		stringstream ss;
		ss << "SELECT relation FROM relations WHERE objecttype1 = " << static_cast<int>(objectType)
			<< " AND objectid1 = " << static_cast<int>(objectID) << " ORDER BY priority ASC;";
		string s(ss.str());
		int rc = sqlite3_exec(db, s.c_str(), CallbackStringVector, &relations, &dbError);
		if (rc == SQLITE_OK)
		{
			return;
		}

		logger->Error("SQLite error: {0} Query: {1}", dbError, s);
		sqlite3_free(dbError);
	}

	// read datamodelrelations
	void SQLite::RelationsTo(const objectType_t objectType, const objectID_t objectID, vector<string>& relations)
	{
		if (db == nullptr)
		{
			return;
		}

		char* dbError = 0;
		stringstream ss;
		ss << "SELECT relation FROM relations WHERE objecttype2 = " << static_cast<int>(objectType)
			<< " AND objectid2 = " << static_cast<int>(objectID) << ";";
		string s(ss.str());
		int rc = sqlite3_exec(db, s.c_str(), CallbackStringVector, &relations, &dbError);
		if (rc == SQLITE_OK)
		{
			return;
		}

		logger->Error("SQLite error: {0} Query: {1}", dbError, s);
		sqlite3_free(dbError);
	}

	void SQLite::SaveSetting(const string& key, const string& value)
	{
		Execute("INSERT OR REPLACE INTO settings (key, value) values ('" + key + "', '" + value + "');");
	}

	string SQLite::GetSetting(const string& key)
	{
		if (db == nullptr)
		{
			return "";
		}

		char* dbError = 0;
		vector<string> values;
		string s("SELECT value FROM settings WHERE key = '" + key + "';");
		int rc = sqlite3_exec(db, s.c_str(), CallbackStringVector, &values, &dbError);
		if (rc != SQLITE_OK)
		{
			logger->Error("SQLite error: {0} Query: {1}", dbError, s);
			sqlite3_free(dbError);
			return "";
		}
		if (values.size() == 0)
		{
			return "";
		}
		return values[0];
	}

	// callback read all datamodelobjects/relations
	int SQLite::CallbackStringVector(void* v, int argc, char **argv, char **colName)
	{
		vector<string>* objects = static_cast<vector<string>*>(v);
		if (argc != 1)
		{
			return 0;
		}
		objects->push_back(argv[0]);
		return 0;
	}

	void SQLite::Execute(const string& s)
	{
		if (db == nullptr)
		{
			return;
		}

		char* dbError = nullptr;
		int rc = sqlite3_exec(db, s.c_str(), nullptr, nullptr, &dbError);
		if (rc == SQLITE_OK)
		{
			return;
		}

		logger->Error("SQLite error: {0} Query: {1}", dbError, s);
		sqlite3_free(dbError);
	}
} // namespace storage

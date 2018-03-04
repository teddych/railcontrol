#include <map>
#include <sstream>

#include "sqlite.h"
#include "util.h"

using datamodel::Accessory;
using datamodel::Block;
using datamodel::Feedback;
using datamodel::Loco;
using datamodel::Switch;
using hardware::HardwareParams;
using std::map;
using std::string;
using std::stringstream;
using std::vector;

namespace storage {

	// create instance of sqlite
	extern "C" SQLite* create_sqlite(const StorageParams& params) {
		return new SQLite(params);
  }

	// delete instance of sqlite
  extern "C" void destroy_sqlite(SQLite* sqlite) {
    delete(sqlite);
  }

	SQLite::SQLite(const StorageParams& params) {
		int rc;
		char* dbError = NULL;

		xlog("Loading SQLite database with filename %s", params.filename.c_str());
		rc = sqlite3_open(params.filename.c_str(), &db);
		if (rc) {
			xlog("Unable to load SQLite database: %s", sqlite3_errmsg(db));
			sqlite3_close(db);
			db = NULL;
			return;
		}

		// check if needed tables exist
		map<string, bool> tablenames;
		rc = sqlite3_exec(db, "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;", callbackListTables, &tablenames, &dbError);
		if (rc != SQLITE_OK) {
			xlog("SQLite error: %s", dbError);
			sqlite3_free(dbError);
			sqlite3_close(db);
			db = NULL;
			return;
		}

		// create hardware table if needed
		if (tablenames["hardware"] != true) {
			xlog("Creating table hardware");
			rc = sqlite3_exec(db, "CREATE TABLE hardware (controlid UNSIGNED TINYINT PRIMARY KEY, hardwaretype UNSIGNED TINYINT, name VARCHAR(50), ip VARCHAR(46));", NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}

		// create objects table if needed
		if (tablenames["objects"] != true) {
			xlog("Creating table objects");
			rc = sqlite3_exec(db, "CREATE TABLE objects ("
				"objecttype UNSIGNED TINYINT, "
				"objectid UNSIGNED SHORTINT, "
				"name VARCHAR(50), "
				"object SHORTTEXT,"
				"PRIMARY KEY (objecttype, objectid));",
				NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}

		// create relations table if needed
		if (tablenames["relations"] != true) {
			xlog("Creating table relations");
			rc = sqlite3_exec(db, "CREATE TABLE relations ("
				"objecttype1 UNSIGNED TINYINT, "
				"objectid1 UNSIGNED SHORTINT, "
				"objecttype2 UNSIGNED TINYINT, "
				"objectid2 UNSIGNED SHORTINT, "
				"name VARCHAR(50), "
				"relation SHORTTEXT,"
				"PRIMARY KEY (objecttype1, objectid1, objecttype2, objectid2));",
				NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}
	}

	SQLite::~SQLite() {
		if (db) {
			xlog("Closing SQLite database");
			sqlite3_close(db);
			db = NULL;
		}
	}

	int SQLite::callbackListTables(void* v, int argc, char **argv, char **colName) {
		map<string, bool>* tablenames = static_cast<map<string, bool>*>(v);
		(*tablenames)[argv[0]] = true;
		return 0;
	}

	void SQLite::hardwareParams(const hardware::HardwareParams& hardwareParams) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			ss << "INSERT OR REPLACE INTO hardware VALUES (" << (int)hardwareParams.controlID << ", " << (int)hardwareParams.hardwareType << ", '" << hardwareParams.name << "', '" << hardwareParams.ip << "');";
			int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	void SQLite::allHardwareParams(std::map<controlID_t,hardware::HardwareParams*>& hardwareParams) {
		if (db) {
			char* dbError = 0;
			int rc = sqlite3_exec(db, "SELECT controlid, hardwaretype, name, ip FROM hardware ORDER BY controlid;", callbackAllHardwareParams, &hardwareParams, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// callback read hardwareparams
	int SQLite::callbackAllHardwareParams(void* v, int argc, char **argv, char **colName) {
		map<controlID_t,HardwareParams*>* hardwareParams = static_cast<map<controlID_t,HardwareParams*>*>(v);
		if (argc != 4) {
			return 0;
		}
		controlID_t controlID = atoi(argv[0]);
		if (hardwareParams->count(controlID)) {
			xlog("Control with ID %i already exists", controlID);
		}
		HardwareParams* params = new HardwareParams(controlID, atoi(argv[1]), argv[2], argv[3]);
		(*hardwareParams)[controlID] = params;
		return 0;
	}

	// delete control
	void SQLite::deleteHardwareParams(const controlID_t controlID) {
		if (db == nullptr) {
			return;
		}
		stringstream ss;
		ss << "DELETE FROM hardware WHERE controlid = " << (int)controlID << ";";
		string s(ss.str());
		char* dbError = NULL;
		int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &dbError);
		if (rc == SQLITE_OK) {
			return;
		}
		xlog("SQLite error: %s Query: %s", dbError, s.c_str());
		sqlite3_free(dbError);
	}

	// save datamodelobject
	void SQLite::saveObject(const objectType_t objectType, const objectID_t objectID, const std::string& name, const std::string& object) {
		if (db == nullptr) {
			return;
		}
		stringstream ss;
		char* dbError = NULL;
		// FIXME: escape "'" in object
		ss << "INSERT OR REPLACE INTO objects (objecttype, objectid, name, object) VALUES (" << (int)objectType << ", " << (int)objectID << ", '" << name << "', '" << object << "');";
		string s(ss.str());
		int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &dbError);
		if (rc == SQLITE_OK) {
			return;
		}

		xlog("SQLite error: %s Query: %s", dbError, s.c_str());
		sqlite3_free(dbError);
	}

	// delete datamodelobject
	void SQLite::deleteObject(const objectType_t objectType, const objectID_t objectID) {
		if (db == nullptr) {
			return;
		}
		stringstream ss;
		ss << "DELETE FROM objects WHERE objecttype = " << (int)objectType << " AND objectid = " << (int)objectID << ";";
		string s(ss.str());
		char* dbError = NULL;
		int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &dbError);
		if (rc == SQLITE_OK) {
			return;
		}
		xlog("SQLite error: %s Query: %s", dbError, s.c_str());
		sqlite3_free(dbError);
	}

	// read datamodelobjects
	void SQLite::objectsOfType(const objectType_t objectType, vector<string>& objects) {
		if (db) {
			char* dbError = 0;
			stringstream ss;
			ss << "SELECT object FROM objects WHERE objecttype = " << (int)objectType << " ORDER BY objectid;";
			string s(ss.str());
			int rc = sqlite3_exec(db, s.c_str(), callbackStringVector, &objects, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// save datamodelrelation
	void SQLite::saveRelation(const relationType_t relationType, const objectID_t objectID1, const objectID_t objectID2, const std::string& name, const std::string& relation) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			// FIXME: escape "'" in object
			ss << "INSERT OR REPLACE INTO relations (relationtype, objectid1, objectid2, name, relation) VALUES (" << (int)relationType << ", " << (int)objectID1 << ", " << (int)objectID2 << ", '" << name << "', '" << relation << "');";
			string s(ss.str());
			int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// read datamodelrelations
	void SQLite::relationsFromObject(const relationType_t relationType, const objectID_t objectID, vector<string>& relations) {
		if (db) {
			char* dbError = 0;
			stringstream ss;
			ss << "SELECT relation FROM relations WHERE relationtype = " << (int)relationType << " AND objectid1 = " << (int)objectID << ";";
			string s(ss.str());
			int rc = sqlite3_exec(db, s.c_str(), callbackStringVector, &relations, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// read datamodelrelations
	void SQLite::relationsToObject(const relationType_t relationType, const objectID_t objectID, vector<string>& relations) {
		if (db) {
			char* dbError = 0;
			stringstream ss;
			ss << "SELECT relation FROM relations WHERE relationtype = " << (int)relationType << " AND objectid2 = " << (int)objectID << ";";
			string s(ss.str());
			int rc = sqlite3_exec(db, s.c_str(), callbackStringVector, &relations, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// callback read all datamodelobjects
	int SQLite::callbackStringVector(void* v, int argc, char **argv, char **colName) {
		vector<string>* objects = static_cast<vector<string>*>(v);
		if (argc != 1) {
			return 0;
		}
		objects->push_back(argv[0]);
		return 0;
	}

} // namespace storage

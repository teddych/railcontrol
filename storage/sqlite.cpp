#include <map>
#include <sstream>

#include "sqlite.h"
#include "util.h"

using datamodel::Accessory;
using datamodel::Block;
using datamodel::Feedback;
using datamodel::Loco;
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
			rc = sqlite3_exec(db, "CREATE TABLE hardware (controlid UNSIGNED TINYINT PRIMARY KEY, hardwareid UNSIGNED TINYINT, name VARCHAR(50), ip VARCHARi(46));", NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}

		// create loco table if needed
		if (tablenames["locos"] != true) {
			xlog("Creating table locos");
			rc = sqlite3_exec(db, "CREATE TABLE locos (locoid UNSIGNED INT PRIMARY KEY, name VARCHAR(50), controlid UNSIGNED TINYINT, protocol UNSIGNED TINYINT, address UNSIGNED SHORTINT);", NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}

		// create accessories table if needed
		if (tablenames["accessories"] != true) {
			xlog("Creating table accessories");
			rc = sqlite3_exec(db, "CREATE TABLE accessories (accessoryid UNSIGNED INT PRIMARY KEY, name VARCHAR(50), controlid UNSIGNED TINYINT, protocol UNSIGNED TINYINT, address UNSIGNED SHORTINT, type UNSIGNED TINYINT);", NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}

		// create feedbacks table if needed
		if (tablenames["feedbacks"] != true) {
			xlog("Creating table feedbacks");
			rc = sqlite3_exec(db, "CREATE TABLE feedbacks (feedbackid UNSIGNED INT PRIMARY KEY, name VARCHAR(50), controlid UNSIGNED TINYINT, pin UNSIGNED INT);", NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
				sqlite3_close(db);
				db = NULL;
				return;
			}
		}

		// create blocks table if needed
		if (tablenames["blocks"] != true) {
			xlog("Creating table blocks");
			rc = sqlite3_exec(db, "CREATE TABLE blocks (blockid UNSIGNED INT PRIMARY KEY, name VARCHAR(50));", NULL, NULL, &dbError);
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
			ss << "INSERT OR REPLACE INTO hardware VALUES (" << (int)hardwareParams.controlID << ", " << (int)hardwareParams.hardwareID << ", '" << hardwareParams.name << "', '" << hardwareParams.ip << "');";
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
			int rc = sqlite3_exec(db, "SELECT controlid, hardwareid, name, ip FROM hardware ORDER BY controlid;", callbackAllHardwareParams, &hardwareParams, &dbError);
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


	// save loco (locoID is primary key)
	void SQLite::loco(const datamodel::Loco& loco) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			ss << "INSERT OR REPLACE INTO locos (locoid, name, controlid, protocol, address) VALUES (" << loco.locoID << ", '" << loco.name << "', " << (int)loco.controlID << ", " << (int)loco.protocol << ", " << loco.address << ");";
			int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// read all locos
	void SQLite::allLocos(std::map<locoID_t, datamodel::Loco*>& locos) {
		if (db) {
			char* dbError = 0;
			int rc = sqlite3_exec(db, "SELECT locoid, name, controlid, protocol, address FROM locos ORDER BY locoid;", callbackAllLocos, &locos, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// callback read all locos
	int SQLite::callbackAllLocos(void* v, int argc, char **argv, char **colName) {
		map<locoID_t,Loco*>* locos = static_cast<map<locoID_t,Loco*>*>(v);
		if (argc != 5) {
			return 0;
		}
		locoID_t locoID = atoi(argv[0]);
		if (locos->count(locoID)) {
			xlog("Loco with ID %i already exists", locoID);
			Loco* loco = (*locos)[locoID];
			delete loco;
		}
		Loco* loco = new Loco(locoID, argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));

		(*locos)[locoID] = loco;
		return 0;
	}

	// save accessory
	void SQLite::accessory(const datamodel::Accessory& accessory) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			ss << "INSERT OR REPLACE INTO accessories (accessoryid, name, controlid, protocol, address, type) VALUES (" << accessory.accessoryID << ", '" << accessory.name << "', " << (int)accessory.controlID << ", " << (int)accessory.protocol << ", " << accessory.address << ", " << (int)accessory.type << ");";
			int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// read all accessories
	void SQLite::allAccessories(std::map<accessoryID_t,datamodel::Accessory*>& accessories) {
			if (db) {
			char* dbError = 0;
			int rc = sqlite3_exec(db, "SELECT accessoryid, name, controlid, protocol, address, type FROM accessories ORDER BY accessoryid;", callbackAllAccessories, &accessories, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// callback read all accessories
	int SQLite::callbackAllAccessories(void* v, int argc, char **argv, char **colName) {
		map<accessoryID_t,Accessory*>* accessories = static_cast<map<accessoryID_t,Accessory*>*>(v);
		if (argc != 6) {
			return 0;
		}
		accessoryID_t accessoryID = atoi(argv[0]);
		if (accessories->count(accessoryID)) {
			xlog("Accessory with ID %i already exists", accessoryID);
			Accessory* accessory = (*accessories)[accessoryID];
			delete accessory;
		}
		Accessory* accessory = new Accessory(accessoryID, argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));

		(*accessories)[accessoryID] = accessory;
		return 0;
	}

	// save feedback
	void SQLite::feedback(const datamodel::Feedback& feedback) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			ss << "INSERT OR REPLACE INTO feedbacks (feedbackid, name, controlid, pin) VALUES (" << feedback.feedbackID << ", '" << feedback.name << "', " << (int)feedback.controlID << ", " << feedback.pin << ");";
			int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// read all feedbacks
	void SQLite::allFeedbacks(std::map<feedbackID_t,datamodel::Feedback*>& feedbacks) {
			if (db) {
			char* dbError = 0;
			int rc = sqlite3_exec(db, "SELECT feedbackid, name, controlid, pin FROM feedbacks ORDER BY feedbackid;", callbackAllFeedbacks, &feedbacks, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// callback read all feedbacks
	int SQLite::callbackAllFeedbacks(void* v, int argc, char **argv, char **colName) {
		map<feedbackID_t,Feedback*>* feedbacks = static_cast<map<feedbackID_t,Feedback*>*>(v);
		if (argc != 4) {
			return 0;
		}
		feedbackID_t feedbackID = atoi(argv[0]);
		if (feedbacks->count(feedbackID)) {
			xlog("feedback with ID %i already exists", feedbackID);
			Feedback* feedback = (*feedbacks)[feedbackID];
			delete feedback;
		}
		Feedback* feedback = new Feedback(feedbackID, argv[1], atoi(argv[2]), atoi(argv[3]));

		(*feedbacks)[feedbackID] = feedback;
		return 0;
	}

	// save block
	void SQLite::block(const datamodel::Block& block) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			ss << "INSERT OR REPLACE INTO blocks (blockid, name) VALUES (" << block.blockID << ", '" << block.name << "');";
			int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// read all blocks
	void SQLite::allBlocks(std::map<blockID_t,datamodel::Block*>& blocks) {
			if (db) {
			char* dbError = 0;
			int rc = sqlite3_exec(db, "SELECT blockid, name FROM blocks ORDER BY blockid;", callbackAllBlocks, &blocks, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	// callback read all blocks
	int SQLite::callbackAllBlocks(void* v, int argc, char **argv, char **colName) {
		map<blockID_t,Block*>* blocks = static_cast<map<blockID_t,Block*>*>(v);
		if (argc != 2) {
			return 0;
		}
		blockID_t blockID = atoi(argv[0]);
		if (blocks->count(blockID)) {
			xlog("Block with ID %i already exists", blockID);
			Block* block = (*blocks)[blockID];
			delete block;
		}
		Block* block = new Block(blockID, argv[1]);

		(*blocks)[blockID] = block;
		return 0;
	}

} // namespace storage


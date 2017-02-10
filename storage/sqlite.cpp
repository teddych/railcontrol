#include <map>
#include <sstream>

#include "sqlite.h"
#include "util.h"

using std::map;
using std::string;
using std::stringstream;
using std::vector;
using datamodel::Loco;

namespace storage {

	// create instance of sqlite
	extern "C" SQLite* create_sqlite(const StorageParams& params) {
		return new SQLite(params);
  }

	// delete instance of sqlite
  extern "C" void destroy_sqlite(SQLite* sqlite) {
    delete(sqlite);
  }

	SQLite::SQLite(const StorageParams& params) :
		params(params) {
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

		if (tablenames["locos"] != true) {
			xlog("Creating table locos");
			rc = sqlite3_exec(db, "CREATE TABLE locos (locoid UNSIGNED INT PRIMARY KEY, name VARCHAR(50), protocol UNSIGNED TINYINT, address UNSIGNED SHORTINT);", NULL, NULL, &dbError);
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
			xlog("Closing SQLite database with filename %s", params.filename.c_str());
			sqlite3_close(db);
			db = NULL;
		}
	}

	int SQLite::callbackListTables(void* v, int argc, char **argv, char **colName) {
		map<string, bool>* tablenames = static_cast<map<string, bool>*>(v);
		(*tablenames)[argv[0]] = true;
		return 0;
	}

	// save loco (locoID is primary key)
	void SQLite::loco(const datamodel::Loco& loco) {
		if (db) {
			stringstream ss;
			char* dbError = NULL;
			ss << "INSERT OR REPLACE INTO locos VALUES (" << loco.locoID << ", '" << loco.name << "', " << (int)loco.protocol << ", " << loco.address << ");";
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
			int rc = sqlite3_exec(db, "SELECT locoid, name, protocol, address FROM locos ORDER BY locoid;", callbackAllLocos, &locos, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
		}
	}

	int SQLite::callbackAllLocos(void* v, int argc, char **argv, char **colName) {
		map<locoID_t,Loco*>* locos = static_cast<map<locoID_t,Loco*>*>(v);
		if (argc != 4) {
			return 0;
		}
		locoID_t locoID = atoi(argv[0]);
		if (locos->count(locoID)) {
			xlog("Loco with ID %i already exists", locoID);
			Loco* loco = (*locos)[locoID];
			delete loco;
		}
		Loco* loco = new Loco(locoID, argv[1], atoi(argv[2]), atoi(argv[3]));

		(*locos)[locoID] = loco;
		return 0;
	}

} // namespace storage


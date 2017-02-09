#include <map>
#include <iostream>

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
			rc = sqlite3_exec(db, "CREATE TABLE locos (locoid UNSIGNED INT, name VARCHAR(50), protocol UNSIGNED TINYINT, address UNSIGNED SHORTINT);", NULL, NULL, &dbError);
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
		int i;
		for (i = 0; i < argc; ++i) {
			xlog("%s = %s", colName[i], argv[i]);
			(*tablenames)[argv[i]] = true;
		}
		return 0;
	}

	// save loco
	void SQLite::loco(const datamodel::Loco& loco) {
		if (db) {
		/*
			stringstream ss;
			ss << "INSERT INTO locos VALUES (" << loco.id << ", '" << loco.name << "', " << loco.protocol << ", " << loco.address << ");";
			int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, &dbError);
			if (rc != SQLITE_OK) {
				xlog("SQLite error: %s", dbError);
				sqlite3_free(dbError);
			}
			*/
		}
	}

	// read all locos
	std::vector<datamodel::Loco*> SQLite::allLocos() {
		int i = 1;
		vector<Loco*> locos;

		Loco* l = new Loco(i++);
		l->name = "my Loco";
		l->address = 1200;
		l->protocol = PROTOCOL_DCC;
		locos.push_back(l);

		l = new Loco(i++);
		l->name = "your Loco";
		l->address = 1201;
		l->protocol = PROTOCOL_DCC;
		locos.push_back(l);

		return locos;
	}

} // namespace storage


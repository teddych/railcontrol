#include "sqlite.h"
#include "sqlite/sqlite3.h"
#include "util.h"

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

		xlog("Loading SQLite database with filename %s", params.filename.c_str());
	}

	SQLite::~SQLite() {
		xlog("Closing SQLite database with filename %s", params.filename.c_str());
	}

	// save loco
	void SQLite::loco(const datamodel::Loco& loco) {
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


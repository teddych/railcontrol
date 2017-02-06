#include "sqlite.h"
#include "sqlite/sqlite3.h"

using std::vector;
using datamodel::Loco;

namespace storage {

SQLite::SQLite(StorageParams& params) {
}

// save loco
void SQLite::loco(const datamodel::Loco& loco) {
}

// read all locos
std::vector<datamodel::Loco*> allLocos() {
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


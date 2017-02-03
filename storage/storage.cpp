#include "storage.h"

using datamodel::Loco;
using std::vector;

namespace storage {

Storage::Storage() {
}

Storage::~Storage() {
}

void Storage::loco(const Loco& loco) {
}

vector<Loco*> Storage::allLocos() {
	locoID_t i = 0;
	vector<Loco*> locos;
	Loco* l = new Loco(i++);
	l->name = "myLoco";
	locos.push_back(l);
	return locos;
}

} // namespace storage


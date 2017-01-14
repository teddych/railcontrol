#ifndef HARDWARE_H
#define HARDWARE_H

#include <map>

typedef unsigned int hardware_id_t;

enum hardware_ids {
  HARDWARE_ID_VIRT = 0,
	HARDWARE_ID_CS2,
	HARDWARE_ID_NUM
};

static std::string hardware_symbols[] = {
	"virt",
	"cs2"
};

#endif // HARDWARE_H

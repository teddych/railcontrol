#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "control.h"

class manager {
	public:
    manager();
    ~manager();
	private:
		void loco_speed(const control_id_t control_id, const loco_id_t loco_id, const speed_t speed);
    std::vector<control*> controllers;
};

#endif // MANAGER_H


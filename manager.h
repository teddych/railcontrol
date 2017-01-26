#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "control.h"

class manager {
	public:
    manager();
    ~manager();
	private:
		void loco_speed(unsigned int control_id, unsigned char protocol, unsigned short address, int speed);
    std::vector<control> controllers;
};

#endif // MANAGER_H


#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "control.h"

class Manager {
	public:
    Manager();
    ~Manager();
		void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed);
	private:
    std::vector<Control*> controllers;
};

#endif // MANAGER_H


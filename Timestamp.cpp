#include <ctime>

#include "Timestamp.h"

time_t GetCompileTime()
{
	return __COMPILE_TIME__;
}


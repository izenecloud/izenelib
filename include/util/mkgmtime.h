#ifndef _MKGMTIME_H_
#define _MKGMTIME_H_

#include <time.h>
time_t mkgmtime(tm *tim_p);

time_t fastmktime(tm *tim_p);

#endif //_MKGMTIME_H_


#ifndef PROFILER_H_H_
#define PROFILER_H_H_

#include "Profiler.h"

NS_IZENELIB_UTIL_BEGIN

#ifdef SF1_MEMCHECK
typedef Profiler0 Profiler;
#else

typedef Profiler1 Profiler;
#endif

NS_IZENELIB_UTIL_END



#endif /*PROFILER_H_H_*/

#ifndef IZENE_LOG_H_
#define IZENE_LOG_H_

#include <glog/logging.h>
#include <util/profiler/ProfilerGroup.h>

#include "ProcMemInfo.h"
#include <sstream>


#ifdef _DEBUG
#define _DEBUG_NEW_REDEFINE_NEW 0
#include <nvwa/debug_new.h>
#else
#define DEBUG_NEW new
#endif


#define MEMLOG(...) \
{ \
char c[500]; \
sprintf(c, __VA_ARGS__ ); \
std::string output(c); \
sprintf(c, " /MEM: %d/", izenelib::util::GetMemNum()); \
std::string mem(c); \
output += mem; \
LOG(INFO)<<output; \
} \

NS_IZENELIB_UTIL_BEGIN


std::string  getMemInfo();
std::string getProfilingInfo();
int GetMemNum();

NS_IZENELIB_UTIL_END


#endif /*IZENE_LOG_H_*/

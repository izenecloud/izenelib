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


NS_IZENELIB_UTIL_BEGIN


std::string  getMemInfo();
std::string getProfilingInfo();
unsigned long GetMemNum();




NS_IZENELIB_UTIL_END


#endif /*IZENE_LOG_H_*/

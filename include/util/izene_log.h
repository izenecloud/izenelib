#ifndef IZENE_LOG_H_
#define IZENE_LOG_H_

#define GOOGLE_STRIP_LOG 1    // this must go before the #include!


#include <glog/logging.h>

#ifdef SF1_TIME_CHECK
	#include <wiselib/profiler/ProfilerGroup.h>
#endif

#include "ProcMemInfo.h"
#include <sstream>


#ifdef _DEBUG
#define _DEBUG_NEW_REDEFINE_NEW 0
#include <nvwa/debug_new.h>
#else
#define DEBUG_NEW new
#endif

using namespace std;

NS_IZENELIB_UTIL_BEGIN

string  getMemInfo(); 
string getProfilingInfo();


NS_IZENELIB_UTIL_END


#endif /*IZENE_LOG_H_*/

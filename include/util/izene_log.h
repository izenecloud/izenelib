#ifndef IZENE_LOG_H_
#define IZENE_LOG_H_

#define GOOGLE_STRIP_LOG 1    // this must go before the #include!
#include <glog/logging.h>

#include "ProcMemInfo.h"
#include <sstream>

using namespace std;


NS_IZENELIB_UTIL_BEGIN

string  getMemInfo() {
	std::stringstream ss;
	unsigned long rlimit;
	static unsigned long  vm = 0, rss;
	unsigned long pre_vm = vm;
	unsigned long pre_rss = rss;
	
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
	ss << "Current vm: " << vm << " bytes; rss: " << rss << " bytes.  " ;
	if(vm > pre_vm )
		ss << "Increased vm: " << vm - pre_vm << " bytes; rss: " << rss - pre_rss << " bytes." << endl;
	else 
		ss << "Decreased vm: " <<  pre_vm - vm << " bytes; rss: " << pre_rss -rss  << " bytes." << endl;
	
	return ss.str();
}


NS_IZENELIB_UTIL_END


#endif /*IZENE_LOG_H_*/

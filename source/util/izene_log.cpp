#include <util/izene_log.h>


NS_IZENELIB_UTIL_BEGIN

std::string  getMemInfo() {
	std::stringstream ss;
	unsigned long rlimit;
	static unsigned long  vm = 0, rss;
	unsigned long pre_vm = vm;
	unsigned long pre_rss = rss;

    sleep(2);
	ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
    
	ss << "Current vm(virtual memory): " << vm << " bytes; \t rss(Resident Set Size): " << rss << " bytes. " ;
	if(vm >= pre_vm )
		ss << "\t ++++ vm: " << vm - pre_vm << " bytes;";				
	else 
		ss << "\t ---- vm: " <<  pre_vm - vm << " bytes;";
		
	if( rss >= pre_rss )
		ss <<" \t ++++ rss: " << rss - pre_rss  << " bytes.";
	else
		ss <<" \t ---- rss: " << pre_rss - rss  << " bytes.";
	ss <<endl;	
	
	return ss.str();
}

int GetMemNum()
{
  unsigned long rlimit =0, vm = 0, rss =0;
  ProcMemInfo::getProcMemInfo(vm, rss, rlimit);
  return (int)(rss/1024/1024);
}

std::string getProfilingInfo(){
	std::stringstream ss;
#ifdef SF1_TIME_CHECK
	REPORT_PROFILE_TO_SS( ss );
	return "\n\n" + ss.str() + "\n\n";
#else
	return "SF1_TIME_CHECK not defined!";
#endif
}


NS_IZENELIB_UTIL_END


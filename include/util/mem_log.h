#ifndef IZENE_MEMLOG_H_
#define IZENE_MEMLOG_H_

#include "izene_log.h"

NS_IZENELIB_UTIL_BEGIN

class MemLog
{
public:
static void Log0(const std::string& format) { 
  std::string the_format(format+" /MEM: %d/"); 
  char info[500]; 
  sprintf(info, the_format.c_str(), izenelib::util::GetMemNum()); 
  std::string output(info); 
  LOG(INFO)<<output; 
}

static void Log1(const std::string& format, std::size_t intparam) {
  std::string the_format(format+" /MEM: %d/"); 
  char info[500]; 
  sprintf(info, the_format.c_str(), intparam, izenelib::util::GetMemNum()); 
  std::string output(info); 
  LOG(INFO)<<output;
}

static void Log2(const std::string& format,std::size_t intparam1,std::size_t intparam2) {
  std::string the_format(format+" /MEM: %d/"); 
  char info[500]; 
  sprintf(info, the_format.c_str(), intparam1, intparam2, izenelib::util::GetMemNum()); 
  std::string output(info); 
  LOG(INFO)<<output;
}
};

NS_IZENELIB_UTIL_END


#endif 


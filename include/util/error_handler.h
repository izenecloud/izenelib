#ifndef IZENELIB_UTIL_ERRORHANDLER_H_
#define IZENELIB_UTIL_ERRORHANDLER_H_


#include <string>
#include <iostream>
#include <types.h>
NS_IZENELIB_UTIL_BEGIN

class ErrorHandler
{
public:
  ErrorHandler():last_error_id_(0)
  {}
  ~ErrorHandler()
  {}
  
  bool GetLastError() const
  {
    if(last_error_id_==0) return false;
    return true;
  }
  
  bool GetLastError(uint8_t& id) const
  {
    if(last_error_id_==0) return false;
    id = last_error_id_;
    return true;
  }
  
  bool GetLastError(uint8_t& id, std::string& msg) const
  {
    if(last_error_id_==0) return false;
    id = last_error_id_;
    msg = last_error_msg_;
    return true;
  }
  
protected:
  
  void RegisterErrorMsg_(uint8_t id, const std::string& msg)
  {
    error_.insert(id, msg);
  }
  
  void ReportError_(uint8_t id)
  {
    std::string msg;
    if( error_.get(id, msg) )
    {
      last_error_id_ = id;
      last_error_msg_ = msg;
    }
  }
  
  void ResetError_()
  {
    last_error_id_ = 0;
  }
  
protected:
  izenelib::am::rde_hash<uint8_t, std::string> error_;
  uint8_t last_error_id_;
  std::string last_error_msg_;
        
};

   
NS_IZENELIB_UTIL_END



#endif /* SF1V5_FILEOBJECT_H_ */

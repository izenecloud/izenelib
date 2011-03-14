#ifndef IZENELIB_UTIL_FILEOBJECT_H_
#define IZENELIB_UTIL_FILEOBJECT_H_


#include <string>
#include <iostream>
#include <fstream>
#include <types.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem.hpp>
NS_IZENELIB_UTIL_BEGIN
    
template <typename T>
class FileObject
{
public:
  FileObject(const std::string& file):file_(file)
  {}
  ~FileObject()
  {}
  
  void SetValue(const T& tvalue)
  {
    value = tvalue;
  }
  
  T GetValue()
  {
    return value;
  }
  
  
  bool Save()
  {
    std::ofstream ofs(file_.c_str());
    if( ofs.fail()) return false;
    {
      boost::archive::text_oarchive oa(ofs);
      oa << value ;
    }
    ofs.close();
    if( ofs.fail() )
    {
      return false;
    }
    return true;
  }
  
  bool Load()
  {
    try
    {
      std::ifstream ifs(file_.c_str(), std::ios::binary);
      if( ifs.fail()) return false;
      {
        boost::archive::text_iarchive ia(ifs);
        ia >> value ;
      }
      ifs.close();
      if( ifs.fail() )
      {
        return false;
      }
    }
    catch(std::exception& ex)
    {
      std::cerr<<ex.what()<<std::endl;
      return false;
    }
    return true;
  }
  
public:
  T value;
  
private:
  std::string file_;
        
};

   
NS_IZENELIB_UTIL_END



#endif 

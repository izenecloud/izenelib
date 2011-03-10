#ifndef IZENELIB_AM_MATRIXMEMIO_H_
#define IZENELIB_AM_MATRIXMEMIO_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <util/file_object.h>
NS_IZENELIB_AM_BEGIN

template <typename VT, typename I = uint32_t>
class MatrixMemIo
{
public:

  MatrixMemIo(const std::string& dir):dir_(dir), storage_(NULL)
  {
  }
  ~MatrixMemIo()
  {
    if(storage_!=NULL)
    {
      delete storage_;
    }
  }
  
  bool Open()
  {
    try
    {
      boost::filesystem::create_directories(dir_);
      std::string storage_file = dir_+"/storage";
      storage_ = new idmlib::util::FileObject<std::vector<VT> >(storage_file);
      if(!storage_->Load())
      {
        return false;
      }
    }
    catch(std::exception& ex)
    {
      std::cout<<ex.what()<<std::endl;
      return false;
    }
    return true;
  }
  
  bool Flush()
  {
    return storage_->Save();
  }
  
  
  bool GetVector(I id, VT& vec)
  {
    if(id<storage_->value.size())
    {
      vec = storage_->value[id];
      return true;
    }
    return false;
  }
  
  bool SetVector(I id, const VT& vec)
  {
    if(id>=storage_->value.size())
    {
      storage_->value.resize(id+1);
    }
    storage_->value[id] = vec;
    return true;
  }
  
  void Resize(I isize)
  {
    storage_->value.resize(isize);
  }
  
 
 private: 
  std::string dir_;
  izenelib::util::FileObject<std::vector<VT> >* storage_;
//   FunctionType callback_;
};

   
NS_IZENELIB_AM_END



#endif 

#ifndef IZENELIB_AM_MATRIXMEMIO_H_
#define IZENELIB_AM_MATRIXMEMIO_H_


#include <string>
#include <iostream>
#include <types.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <util/file_object.h>
NS_IZENELIB_AM_BEGIN

/// starts with id = 1
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
      storage_ = new izenelib::util::FileObject<std::vector<VT> >(storage_file);
      storage_->Load();
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
  
  VT& GetVector(I id)
  {
    return storage_->value[id-1];
  }
  
  bool GetVector(I id, VT& vec)
  {
    if(id==0) return false;
    if(id<=storage_->value.size())
    {
      vec = storage_->value[id-1];
      return true;
    }
    return false;
  }
  
  bool SetVector(I id, const VT& vec)
  {
    if(id==0) return false;
    if(id>storage_->value.size())
    {
      storage_->value.resize(id);
    }
    storage_->value[id-1] = vec;
    return true;
  }
  
  
  void Resize(I isize)
  {
    storage_->value.resize(isize);
  }
  
  I VectorCount()
  {
    return storage_->value.size();
  }
  
 
 protected: 
  std::string dir_;
  izenelib::util::FileObject<std::vector<VT> >* storage_;
//   FunctionType callback_;
};


   
NS_IZENELIB_AM_END



#endif 

#ifndef IZENELIB_AM_MATRIXFILEIO_H_
#define IZENELIB_AM_MATRIXFILEIO_H_


#include <string>
#include <iostream>
#include <types.h>
#include <am/sequence_file/SequenceFile.hpp>
#include <am/tokyo_cabinet/tc_hash.h>
NS_IZENELIB_AM_BEGIN

/// starts with id = 1
template <typename VT, typename I = uint32_t>
class MatrixFileFLIo
{
public:

  MatrixFileFLIo(const std::string& dir):dir_(dir), storage_(NULL)
  {
    
  }
  ~MatrixFileFLIo()
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
      storage_ = new izenelib::am::SequenceFile<VT>(storage_file);
      storage_->open();
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
    storage_->flush();
    return true;
  }
  
  
  bool GetVector(I id, VT& vec)
  {
    return storage_->get(id, vec);
  }
  
  bool SetVector(I id, const VT& vec)
  {
    storage_->update(id, vec);
    return true;
  }
  
  I VectorCount()
  {
    return storage_->getItemCount();
  }
  
 
 private: 
  std::string dir_;
  izenelib::am::SequenceFile<VT>* storage_;
//   FunctionType callback_;
};

/// starts with id = 1
template <typename VT, typename I = uint32_t>
class MatrixFileVLIo
{
public:
  typedef izenelib::am::tc_hash<I, VT> StorageType;
  MatrixFileVLIo(const std::string& dir):dir_(dir), storage_(NULL)
  {
    
  }
  ~MatrixFileVLIo()
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
      storage_ = new StorageType(storage_file);
      if(!storage_->open())
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
    storage_->flush();
    return true;
  }
  
  
  bool GetVector(I id, VT& vec)
  {
    return storage_->get(id, vec);
  }
  
  bool SetVector(I id, const VT& vec)
  {
    storage_->update(id, vec);
    return true;
  }
  
  I VectorCount()
  {
    return storage_->num_items();
  }
  
 
 private: 
  std::string dir_;
  StorageType* storage_;
//   FunctionType callback_;
};

   
NS_IZENELIB_AM_END



#endif 

/**
   @file addr_bucket.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef ADDR_BUCKET1_HPP
#define ADDR_BUCKET1_HPP

#include<types.h>
#include <ostream>
#include <iostream>
#include "dyn_array.hpp"
#include <string>


NS_IZENELIB_AM_BEGIN

/**
   @class FileDataBucket
   @brief this is used to store large data set sequentially and get it sorted.
 **/
template<
  typename VALUE_TYPE = uint64_t,
  uint32_t BUF_SIZE = 1000
  >
class FileDataBucket1
{
  typedef DynArray<VALUE_TYPE> array_t;

  array_t buf_;//!< a vector buffer to store data
  FILE* f_;
  uint64_t num_;
  std::string filenm_;
  uint64_t fetch_i_;//!< it indicate the current position when doing uniform access
  
public:
  FileDataBucket1(const char* nm)
  {
    filenm_ = nm;
    buf_.reserve(BUF_SIZE/sizeof(VALUE_TYPE));
    num_ = 0;
    f_ = NULL;
  }

  /**
     @brief this must be called before adding new data.
   */
  void ready4add()
  {
    f_ = fopen(filenm_.c_str(), "r+");
    if (f_ == NULL)
    {
      f_ = fopen(filenm_.c_str(), "w+");
      IASSERT(f_ != NULL);
      IASSERT(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    }
    else
      IASSERT(fread(&num_, sizeof(uint64_t), 1, f_)==1);

    fseek(f_, 0, SEEK_END);

    buf_.reset();
  }

  /**
     @brief add data at tail
   */
  void push_back(const VALUE_TYPE& v)
  {
    if (buf_.length()>=BUF_SIZE/sizeof(VALUE_TYPE))
    {
      buf_.save(f_);
      buf_.reset();
    }

    buf_.push_back(v);
    ++num_;
  }

  /**
     @breif flush data from buffer to disk
   */
  void flush()
  {
    buf_.save(f_);

    fseek(f_, 0, SEEK_SET);
    IASSERT(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    
    fflush(f_);
    fclose(f_);

    f_ = NULL;
    buf_.reset();
  }

  /**
     @brief This must be called before uniform access
   */
  void ready4fetch()
  {
    if (f_!=NULL)
      fclose(f_);
    
    f_ = fopen(filenm_.c_str(), "r+");

    if (f_ == NULL)
    {
      std::cout<<"can't open file: "<<filenm_<<std::endl;
      return;
    }
    else
      IASSERT(fread(&num_, sizeof(uint64_t), 1, f_)==1);

    fetch_i_ = 0;

    buf_.load(f_);
  }

  /**
     @brief record number
   */
  uint64_t num()const
  {
    return num_;
  }

  /**
     @brief get next data
   */
  VALUE_TYPE next()
  {
    if (fetch_i_ >= buf_.length())
    {
      buf_.load(f_);
      fetch_i_ = 0;
    }

    return buf_.at(fetch_i_++);
  }

  /**
     @brief sort all the data
   */
  void sort()
  {
    if (num_<=1)
      return;

    array_t array;
    array.reserve(num_);
    
    ready4fetch();
    array += buf_;

    uint32_t s = BUF_SIZE/sizeof(VALUE_TYPE);
    s = num_%s==0? num_/s: num_/s+1;
    for (uint32_t i=1; i<s; ++i)
    {      
      buf_.load(f_);
      array += buf_;
    }

    fclose(f_);

    //std::cout<<array.length()<<"++++\n";
    array.sort();
    //std::cout<<array.length()<<"----\n";
    
    //quickSort_(array.data(), 0, array.length()-1);

    //std::cout<<array<<std::endl;
    
    num_ = 0;
    f_ = fopen(filenm_.c_str(), "w+");
    IASSERT(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    fclose(f_);

    ready4add();
    
    for (uint32_t i=0; i<array.length(); ++i)
      push_back(array.at(i));

    flush();
  }

  /**
     @brief dump data file
   */
  void dump()
  {
    if (f_ != NULL)
      fclose(f_);
    f_ = 0;
    remove(filenm_.c_str());
  }
  
}
  ;

NS_IZENELIB_AM_END

#endif

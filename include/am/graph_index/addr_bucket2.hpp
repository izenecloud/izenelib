/**
   @file addr_bucket.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef ADDR_BUCKET_HPP
#define ADDR_BUCKET_HPP

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
class FileDataBucket
{
  typedef DynArray<VALUE_TYPE> array_t;

  array_t buf_;//!< a vector buffer to store data
  FILE* f_;
  uint64_t num_;
  uint64_t fetch_i_;//!< it indicate the current position when doing uniform access
  uint64_t start_pos_;//!<the start positioin of this bucket in data file
  uint64_t cur_pos_;//!< current position
  
public:
  FileDataBucket(FILE* f, uint64_t start_pos)
  {
    f_ = f;
    buf_.reserve(BUF_SIZE/sizeof(VALUE_TYPE));
    num_ = 0;
    start_pos_ = start_pos;
  }

  /**
     @brief this must be called before adding new data.
   */
  void ready4add()
  {
    IASSERT(f_ != NULL);
    fseek(f_, sizeof(uint64_t)+start_pos_, SEEK_SET);

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

    fseek(f_, start_pos_, SEEK_SET);
    IASSERT(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    
    fflush(f_);
    buf_.reset();
  }

  /**
     @brief This must be called before uniform access
   */
  void ready4fetch()
  {
    IASSERT(f_ != NULL);
    fseek(f_, start_pos_, SEEK_SET);
    IASSERT(fread(&num_, sizeof(uint64_t), 1, f_)==1);
    fetch_i_ = 0;

    buf_.load(f_);
    cur_pos_ = ftell(f_);
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
      fseek(f_, cur_pos_, SEEK_SET);
      buf_.load(f_);
      cur_pos_ = ftell(f_);
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
    
    array.sort();
    
    num_ = 0;

    ready4add();
    
    for (uint32_t i=0; i<array.length(); ++i)
      push_back(array.at(i));

    flush();
  }
  
}
  ;

NS_IZENELIB_AM_END

#endif

/**
   @file multi_pass_sort.hpp
   @author Kevin Hu
   @date 2010.01.20
 */
#ifndef IZENE_SORT_HPP
#define IZENE_SORT_HPP

#include <util/log.h>
#include <vector>
#include <string>
#include <types.h>
#include <stdio.h>
#include <time.h>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "sort_runner.hpp"
#include "sort_merger.hpp"
#include <math.h>
#include <sys/time.h>

NS_IZENELIB_AM_BEGIN

/**
   @class MultiPassSort
 **/
template<
  class KEY_TYPE = uint32_t,//pre-key type, indicate the length of the pre-key.
  class LEN_TYPE = uint8_t,//
  bool  COMPARE_ALL = false
>
class IzeneSort
{

  typedef IzeneSort<KEY_TYPE, LEN_TYPE, COMPARE_ALL> self_t;
  typedef SortRunner<KEY_TYPE, LEN_TYPE, COMPARE_ALL> run_t;
  typedef SortMerger<KEY_TYPE, LEN_TYPE, COMPARE_ALL> merge_t;
  
  std::string filenm_;
  uint32_t buf_size_;
  uint32_t buf_num_;
  uint64_t count_;

  run_t* run_;
  merge_t* merge_;
  char* buffer_;
  uint32_t pos_;
  FILE* f_;

  inline void new_buffer_()
  {
    if (!buffer_)
      buffer_ = (char*)malloc(buf_size_);
  }

  inline bool is_buffer_full_(LEN_TYPE len)const
  {
    return pos_ +len+sizeof (LEN_TYPE)>buf_size_;
  }
  
public:
  IzeneSort(const char* filenm, uint32_t buf_size = 100000000, uint32_t buf_num=2)
    :filenm_(filenm), buf_size_(buf_size), buf_num_(buf_num),
     count_(0), run_(NULL), merge_(NULL), buffer_(NULL),
     pos_(0), f_(NULL)
  {
  }

  ~IzeneSort()
  {
    if (run_)
      delete run_;

    if (merge_)
      delete merge_;
    
    if (buffer_)
      free(buffer_);
  }

  void set_buffer_size(uint32_t size)
  {
    buf_size_ = size;
  }
  
  void add_data(LEN_TYPE len, const char* data)
  {
    if (len == 0)
      return;

    if (f_ == NULL)
    {
      f_ = fopen(filenm_.c_str(), "r+");
      if (f_== NULL)
      {
        f_ = fopen(filenm_.c_str(), "w+");
        IASSERT(f_!=NULL);
        count_ = 0;
        IASSERT(fwrite(&count_, sizeof(uint64_t), 1, f_)==1);
      }
      else
        IASSERT(fread(&count_, sizeof(uint64_t), 1, f_)==1);
    }
    
    new_buffer_();
    if (is_buffer_full_(len))
    {
      IASSERT(fwrite(buffer_, pos_, 1, f_)==1);
      pos_ = 0;
    }

    *(LEN_TYPE*)(buffer_+pos_) = len;
    pos_ += sizeof(LEN_TYPE);
    memcpy(buffer_+pos_, data, len);
    pos_ += len;
    
    ++count_;
  }

  void flush()
  {
    if (pos_>0)
    {
      IASSERT(fwrite(buffer_, pos_, 1, f_)==1);
      pos_ = 0;
    }
    if (f_)
    {
      fseek(f_, 0, SEEK_SET);
      IASSERT(fwrite(&count_, sizeof(uint64_t), 1, f_)==1);
      fclose(f_);
      f_ = NULL;
    }

    if (buffer_)
    {
      free(buffer_);
      buffer_ = NULL;
    }
  }
  
  bool sort(const std::string& filenm = "")
  {
    flush();
    struct timeval tvafter, tvpre;
    struct timezone tz;

    gettimeofday (&tvpre , &tz);
    if (!filenm.empty())
      filenm_ = filenm;

    FILE* f = fopen(filenm_.c_str(), "r");
    if(f == NULL)
      return false;
    if (fread(&count_, sizeof(uint64_t), 1, f)!=1)
    {
      fclose(f);
      return false;
    }
    fclose(f);
    
    if (count_ <= 1)
      return true;

    if (run_)
      delete run_;
    
    run_ = new run_t(filenm_.c_str(), buf_size_);
    run_->run();
    delete run_;
    run_ = NULL;
    
    merge_ = new merge_t(filenm_.c_str(), run_->run_num(), buf_size_, buf_num_);
    merge_->run();

    gettimeofday (&tvafter , &tz);
    std::cout<<"\nIt takes "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000.)/60000
             <<" minutes to sort("<<count_<<")\n";
    delete merge_;
    merge_ = NULL;

    return true;

  }

  void clear_files()
  {
    if (boost::filesystem::exists(filenm_))
      boost::filesystem::remove(filenm_);
  }
  
  bool begin()
  {
    if (buffer_==NULL)
    {
      buffer_ = (char*)malloc(buf_size_);
      pos_ = 0;
    }

    if (f_ == NULL)
    {
      f_ = fopen(filenm_.c_str(), "r");
      if(f_==NULL)
        return false;
    }
    fseek(f_, 0, SEEK_SET);
    IASSERT(fread(&count_, sizeof(uint64_t), 1, f_)==1);
    
    return true;
  }

  bool next_data(LEN_TYPE& len, char** data)
  {
    if (!f_)
      return false;
    
    if (fread(&len, sizeof(LEN_TYPE), 1, f_)!=1)
    {
      fclose(f_);
      f_ = NULL;
      return false;
    }

    *data = (char*)malloc(len);
    IASSERT(fread(*data, len, 1, f_)==1);
    
    return true;
  }

  uint64_t item_num()const
  {
    return count_;
  }
}
  ;

NS_IZENELIB_AM_END

#endif

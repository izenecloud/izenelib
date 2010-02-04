/**
   @file sorter.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef SORTER_HPP
#define SORTER_HPP

#include<types.h>
#include <am/external_sort/izene_sort.hpp>
#include<string>

NS_IZENELIB_AM_BEGIN

/**
   @class Sorter
   @brief Merge sort is used here. Put all the data in several bucket evenly.
   Get every bucket sorted. Then, merge all of them into one bucket.
 */
template<
  uint32_t BUF_SIZE = 200000000,//!< size of buffer which is used for access
  class TERM_TYPE = uint32_t//!< type of term
  >
class Sorter
{
  typedef IzeneSort<TERM_TYPE, uint16_t, false> sorter_t;
  
private:
  sorter_t sorter_;
  uint64_t num_;//!< data amount
  uint32_t  max_term_len_;//!< the max length of a phrase
  char* buffer_;
  uint32_t buf_size_;
  
public:
  typedef DynArray<TERM_TYPE> terms_t;
  
public:
  Sorter(const char* nm)
    :sorter_(nm, BUF_SIZE), num_(0), max_term_len_(-1), buffer_(NULL), buf_size_(0)
  {
  }

  ~Sorter()
  {
    if(buffer_)
      free(buffer_);
  }

  /**
     @brief this must be called before adding data
   */
  void ready4add()
  {
    num_ = 0;
    if (buffer_)
      return;
    buf_size_ = 1024;
    buffer_ = (char*)malloc(buf_size_);
  }

  inline void set_max_term_len(uint32_t t)
  {
    max_term_len_ = t;
  }
  
  void add_terms(const terms_t& terms, uint32_t docid)
  {
    uint32_t p = 0;
    for (typename terms_t::size_t i=0; i<terms.length(); ++i)
    {
      uint32_t len = ((terms.length()-i>=max_term_len_)? max_term_len_: (terms.length()-i));
      
      uint16_t s = len*sizeof(TERM_TYPE)+sizeof(uint32_t);
      if (s > buf_size_)
      {
        buffer_ = (char*)realloc(buffer_, s);
        buf_size_ = s;
      }
      
      memcpy(buffer_, terms.data()+i, len*sizeof(TERM_TYPE));
      p = len*sizeof(TERM_TYPE);
      
      *(uint32_t*)(buffer_+p) = docid;

      sorter_.add_data(s, buffer_);
      
      ++num_;
    }
  }

  void flush()
  {
    if(buffer_)
      free(buffer_);
    buffer_ = NULL;
    sorter_.flush();
  }

  /**
     @brief After adding all the data, this is called to get all the data sorted.
     Sorting procedure will take place within every bucket firstly. Then, merge
     all the buckets into one and dump all of them. Call output procedure to write
     them into goal file.
   */
  void sort()
  {
    sorter_.sort();
  }

  /**
     @brief this must be called before uniform access
   */
  bool ready4fetch()
  {
    return sorter_.begin();
  }

  /**
     @brief data amount
   */
  inline uint64_t num()const
  {
    return num_;
  }

  /**
     @brief this is used to access the sorted data uniformly.
   */
  bool next(terms_t& terms, uint32_t& docid)
  {
    char* data = NULL;
    uint16_t len=0;
    
    if(!sorter_.next_data(len, &data))
      return false;
    
    terms.assign(data, len - sizeof(uint32_t));
    uint64_t p = terms.size();
    
    docid = *(uint32_t*)(data+p);

    free(data);

    return true;
  }  

  void clear_files()
  {
    sorter_.clear_files();
  }
  
}
;

NS_IZENELIB_AM_END
#endif

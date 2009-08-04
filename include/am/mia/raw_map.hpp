#ifndef RAW_MAP_HPP
#define RAW_MAP_HPP

#include <string>
#include <types.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <stdio.h>
//#include "int_hash_table.hpp"

NS_IZENELIB_AM_BEGIN

template <
  class size_t = uint32_t
  >
class StringVector
{
  const char* buf_;
  
public:
  inline StringVector()
    :buf_(NULL)
  {
  }

  inline StringVector(const char* buf)
  {
    buf_ = buf;
  }

  inline ~StringVector()
  {
  }

  inline void attach(const char* buf)
  {
    buf_ = buf;
  }

  inline size_t size()const
  {
    return *(size_t*)buf_;
  }

  inline size_t length() const 
  {
    size_t s = size();
    size_t k = 0;
    
    for (size_t i=0; i<s; ++i)
      if (*(buf_+sizeof(size_t)+i) == '\0')
        ++k;
    
    return k;
  }

  inline const char* operator [] (size_t t)const
  { 
    size_t s = size();
    size_t k = 0;
    const char* r = buf_+sizeof(size_t);
    
    for (size_t i=0; i<s; ++i)
    {
      if (*(buf_+sizeof(size_t)+i) == '\0')
        ++k;

      if (k-1 == t)
        return r;
      r = buf_+sizeof(size_t)+i+1;
    }

  }

  class const_iterator
  {
  public:
    const_iterator(const char* p=NULL) : p_(p) {}
    
    ~const_iterator() {}

    // The assignment and relational operators are straightforward
    const_iterator& operator=(const const_iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }
    
    bool operator==(const const_iterator& other) const
    {
      return(p_ == other.p_);
    }

    bool operator < (const const_iterator& other) const
    {
      return(p_ < other.p_ );
    }
    
    bool operator > (const const_iterator& other) const
    {
      return(p_ > other.p_);
    }

    bool operator!=(const const_iterator& other) const
    {
      return(p_ != other.p_);
    }
  
    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator++()
    {
      if (p_ == NULL)
        return *this;

      while (*p_!='\0')
        ++p_;
      ++p_;
      
      return(*this);
    }

    const_iterator operator++(int)
    {
      const_iterator tmp(*this);
      while (*p_!='\0')
        ++p_;
      ++p_;
      return(tmp);
    }
    
    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator--()
    {
      if (p_ == NULL)
        return *this;
      
      p_ -= 2;
      while (*p_ != '\0')
        --p_;
      ++p_;
      
      return(*this);
    }

    const_iterator operator--(int)
    {
      const_iterator tmp(*this);

      p_ -= 2;
      while (*p_ != '\0')
        --p_;
      ++p_;
      
      return(tmp);
    }
  
    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const char* operator*() const
    {
      return (p_);
    }
        
    const_iterator operator + (size_t i)const
    {
      const_iterator tmp(*this);

      for (size_t j=0; j<i;++j)
        ++tmp;
      
      return tmp;
    }

    const_iterator operator - (size_t i)const
    {
      const_iterator tmp(*this);

      for (size_t j=0; j<i;++j)
        --tmp;
      
      return tmp;
    }
  
  private:
    const char* p_;
  };

  const_iterator begin()const
  {
    return const_iterator(buf_+sizeof(size_t));
  }

  const_iterator end()const
  {
    return const_iterator(buf_+sizeof(size_t)+*(size_t*)buf_);
  }
};

template <
  class size_t = uint32_t
  >
class IntVector
{
  const size_t* buf_;
  
public:
  inline IntVector()
    :buf_(NULL)
  {
  }

  inline IntVector(const size_t* buf)
  {
    buf_ = buf;
  }

  inline ~IntVector()
  {
  }

  inline void attach(const size_t* buf)
  {
    buf_= buf;
  }
  
  inline size_t size()const
  {
    return *buf_;
  }

  
  class const_iterator
  {
  public:
    const_iterator(const size_t* p=NULL) : p_(p) {}
    
    ~const_iterator() {}

    // The assignment and relational operators are straightforward
    const_iterator& operator=(const const_iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }
    
    bool operator==(const const_iterator& other) const
    {
      return(p_ == other.p_);
    }

    bool operator < (const const_iterator& other) const
    {
      return(p_ < other.p_ );
    }
    
    bool operator > (const const_iterator& other) const
    {
      return(p_ > other.p_);
    }

    bool operator!=(const const_iterator& other) const
    {
      return(p_ != other.p_);
    }
  
    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator++()
    {
      if (p_ == NULL)
        return *this;

      ++p_;
      
      return(*this);
    }

    const_iterator operator++(int)
    {
      const_iterator tmp(*this);
      
      ++p_;
      return(tmp);
    }
    
    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator--()
    {
      if (p_ == NULL)
        return *this;
      
      --p_;
      
      return(*this);
    }

    const_iterator operator--(int)
    {
      const_iterator tmp(*this);

      --p_;
      
      return(tmp);
    }
  
    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const size_t operator*() const
    {
      return *(p_);
    }
        
    const_iterator operator + (size_t i)const
    {
      const_iterator tmp(*this);

      for (size_t j=0; j<i;++j)
        ++tmp;
      
      return tmp;
    }

    const_iterator operator - (size_t i)const
    {
      const_iterator tmp(*this);

      for (size_t j=0; j<i;++j)
        --tmp;
      
      return tmp;
    }
  
  private:
    const size_t* p_;
  };

  const_iterator begin()const
  {
    return const_iterator(buf_+1);
  }

  const_iterator end()const
  {
    return const_iterator(buf_+ 1 +*buf_);
  }
}
  ;

/**
   @class RawMap,
-------------------------------------------------------
   docid | content_length| term1\0term2\0term3\0.....
   docid | content_length| term1\0term2\0term3\0.....
   .
   .
   .
-------------------------------------------------------
 **/
template<
  uint32_t STR_CACHE_SIZE = 100, //MB must be larger than one document
    uint32_t INT_CACHE_SIZE = 100 //MB must be larger than one document
  >
class RawMap
{
public:
  typedef uint32_t size_t;
  typedef StringVector<size_t> str_vector;
  typedef IntVector<size_t>    int_vector;
  //typedef IntHashTable<1000000, size_t>  HashTable;
  
protected:
  uint8_t* str_buf_;
  size_t* int_buf_;
  size_t str_cache_pos_;
  size_t int_cache_pos_;
  size_t str_start_addr_;
  size_t int_start_addr_;
  size_t count_;
  FILE*  f_str_;
  FILE*  f_pos_;
  //HashTable* ht_;

  inline bool str_cache_full(size_t s)
  {
    return str_cache_pos_ + s > STR_CACHE_SIZE*1000000;
  }

  inline bool int_cache_full(size_t s)
  {
    return int_cache_pos_ + s > INT_CACHE_SIZE*1000000/sizeof(size_t);
  }

  inline bool str_in_mem()
  {
    size_t t = *(size_t*)(str_buf_+str_cache_pos_+sizeof(uint32_t));

    return str_cache_pos_ + t + sizeof(uint32_t)+sizeof(size_t) < STR_CACHE_SIZE*1000000;
  }

  inline bool int_in_mem()
  {
    size_t t = *(int_buf_+int_cache_pos_);

    return int_cache_pos_ + t +1 < INT_CACHE_SIZE*1000000/sizeof(size_t);
  }

  inline void str_load_from(size_t addr)
  {
    fseek(f_str_, addr, SEEK_SET);
    fread(str_buf_, STR_CACHE_SIZE*1000000, 1, f_str_);
    str_start_addr_ = addr;
    str_cache_pos_ = 0;
  }

  inline void int_load_from(size_t addr=0)
  {
    fseek(f_pos_, addr, SEEK_SET);
    fread(int_buf_, INT_CACHE_SIZE*1000000, 1, f_pos_);
    int_start_addr_ = addr;
    int_cache_pos_ = 0;
  }

  inline void str_flush()
  {
    fseek(f_str_, 0, SEEK_SET);
    
    assert(fwrite(&count_, sizeof(size_t), 1, f_str_)==1);
    
    fseek(f_str_, str_start_addr_, SEEK_SET);
    
    assert(fwrite(str_buf_, str_cache_pos_, 1, f_str_)==1);
    
    str_start_addr_ += str_cache_pos_;
  }

  inline void int_flush()
  {
    fseek(f_pos_, int_start_addr_, SEEK_SET);
    assert(fwrite(int_buf_, int_cache_pos_*sizeof(size_t), 1, f_pos_)==1);
    int_start_addr_ += int_cache_pos_*sizeof(size_t);
  }
  
public:
  inline RawMap(const char* filenm)
    :str_buf_(NULL), int_buf_(NULL), str_cache_pos_(0), int_cache_pos_(0),
     str_start_addr_(0), int_start_addr_(0), count_(0), f_str_(NULL), f_pos_(NULL)
  {
    std::string s = filenm;
    s += ".str";
    f_str_ = fopen(s.c_str(), "r+");
    if (f_str_ == NULL)
    {
      f_str_ = fopen(s.c_str(), "w+");
      assert(fwrite(&count_, sizeof(size_t), 1, f_str_)==1);
    }
    else
      assert(fread(&count_, sizeof(size_t), 1, f_str_)==1);
    if (f_str_ == NULL)
    {
      std::cout<<"Can't create file: "<<s<<std::endl;
      return;
    }
    
    s = filenm;
    s += ".pos";
    f_pos_ = fopen(s.c_str(), "r+");
    if (f_pos_ == NULL)
      f_pos_ = fopen(s.c_str(), "w+");
    if (f_pos_ == NULL)
    {
      std::cout<<"Can't create file: "<<s<<std::endl;
      return;
    }

//     s = filenm;
//     s += ".has";
//     ht_ = new HashTable(s.c_str());
  }

  inline ~RawMap()
  {
    if (str_buf_ != NULL)
      free(str_buf_);
    if (int_buf_ != NULL)
      free(int_buf_);

    //delete ht_;
  }

  inline void ready4Append()
  {
    str_cache_pos_ = 0;
    int_cache_pos_ = 0;
    
    fseek(f_str_, 0, SEEK_END);
    str_start_addr_ = ftell(f_str_);
    fseek(f_pos_, 0, SEEK_END);
    int_start_addr_ = ftell(f_pos_);
    
    if (str_buf_==NULL)
      str_buf_ = (uint8_t*)malloc(STR_CACHE_SIZE*1000000);
    
    if (int_buf_==NULL)
      int_buf_ = (uint32_t*)malloc(INT_CACHE_SIZE*1000000);
  }

  inline void ready4FetchByTg(size_t t=0)
  {
    str_cache_pos_ = 0;
    int_cache_pos_ = 0;
    str_start_addr_ = sizeof(size_t);
    int_start_addr_ = 0;
    
    if (str_buf_==NULL)
      str_buf_ = (uint8_t*)malloc(STR_CACHE_SIZE*1000000);
    if (int_buf_==NULL)
      int_buf_ = (size_t*)malloc(INT_CACHE_SIZE*1000000);

    str_load_from(sizeof(size_t));
    int_load_from();

    str_vector tmp1;
    int_vector tmp2;
    for (size_t i=0; i<t; ++i)
      next4Tg(tmp1, tmp2);
  }

  inline void ready4SearchByTg()
  {
    str_cache_pos_ = 0;
    int_cache_pos_ = 0;
    str_start_addr_ = sizeof(size_t);
    int_start_addr_ = 0;
    
    if (str_buf_==NULL)
      str_buf_ = (uint8_t*)malloc(1000000);
    if (int_buf_==NULL)
      int_buf_ = (size_t*)malloc(1000000);
  }

  inline void ready4FetchByDupD(size_t t=0)
  {
    str_cache_pos_ = 0;
    int_cache_pos_ = 0;
    str_start_addr_ = sizeof(size_t);
    int_start_addr_ = 0;
    
    if (str_buf_==NULL)
      str_buf_ = (uint8_t*)malloc(STR_CACHE_SIZE*1000000);
    if (int_buf_!=NULL)
    {
      free(int_buf_);
      int_buf_ = NULL;
    }
    
    str_load_from(sizeof(size_t));
    str_vector v;
    for (size_t i=0; i<t; ++i)
      next4DupD(v);
  }

  inline void append(uint32_t docid, const std::vector<std::string>& strs)
  {
    std::size_t s = 0;

    if (int_cache_full(strs.size()+1))
      int_flush();
    
    if (str_cache_full(s+sizeof(uint32_t)+sizeof(size_t)))
      str_flush();

    //ht_->insert(docid, str_start_addr_+str_cache_pos_, int_start_addr_+int_cache_pos_*sizeof(size_t));
    
    *(int_buf_+int_cache_pos_) = strs.size();
    ++int_cache_pos_;

    size_t j = 0;
    for (std::vector<std::string>::const_iterator i=strs.begin(); i!=strs.end(); ++i, ++j)
    {
      if (false)//drop unessary words
        continue;
      
      s += (*i).length()+1;
      *(int_buf_+int_cache_pos_++) = j;
    }

    if (str_cache_full(s+sizeof(uint32_t)+sizeof(size_t)))
      str_flush();

    *(uint32_t*)(str_buf_ + str_cache_pos_) = docid;
    str_cache_pos_ += sizeof(uint32_t);
    *(size_t*)(str_buf_ + str_cache_pos_) = s;
    str_cache_pos_ += sizeof(size_t);
    
    for (std::vector<std::string>::const_iterator i=strs.begin(); i!=strs.end(); ++i)
    {
      if (false)//drop unessary words
        continue;
      
      memcpy(str_buf_ + str_cache_pos_, (*i).c_str(), (*i).length());
      str_cache_pos_ += (*i).length();
      *(str_buf_+str_cache_pos_) = '\0';
      ++ str_cache_pos_;
    }

    ++count_;
  }

  inline void flush()
  {
    str_flush();
    int_flush();
    //ht_->flush();
  }

  inline uint32_t next4DupD(str_vector& v)
  {
    if (!str_in_mem())
      str_load_from(str_cache_pos_+str_start_addr_);

    v.attach((const char*)(str_buf_+str_cache_pos_+sizeof(uint32_t)));
    
    uint32_t docid = *(uint32_t*)(str_buf_+str_cache_pos_);
    
    str_cache_pos_ += sizeof(uint32_t)+sizeof(size_t) + v.size();
    
    return docid;
  }

//   inline bool get4DupD(uint32_t docid, str_vector& v)
//   {
//     size_t addr1;
//     size_t addr2;
//     if (!ht_->find(docid, addr1, addr2))
//       return false;
    
//     fseek(f_str_, addr1, SEEK_SET);
//     assert(fread(str_buf_, sizeof(uint32_t)+sizeof(size_t), 1, f_str_)==1);
//     size_t s = *(size_t*)(str_buf_+sizeof(uint32_t));

    
//     assert(fread(str_buf_+sizeof(uint32_t)+sizeof(size_t), s, 1, f_str_)==1);
//     v.attach((const char*)(str_buf_+sizeof(uint32_t)));

//     return true;
//   }

//   inline bool get4Tg(uint32_t docid, str_vector& v1, int_vector& v2)
//   {
//     size_t addr1;
//     size_t addr2;
    
//     if (!ht_->find(docid, addr1, addr2))
//       return false;
    
//     fseek(f_str_, addr1, SEEK_SET);
//     assert(fread(str_buf_, sizeof(uint32_t)+sizeof(size_t), 1, f_str_)==1);
//     size_t s = *(size_t*)(str_buf_+sizeof(uint32_t));
    
//     assert(fread(str_buf_+sizeof(uint32_t)+sizeof(size_t), s, 1, f_str_)==1);
//     v1.attach((const char*)(str_buf_+sizeof(uint32_t)));

    
//     fseek(f_pos_, addr2, SEEK_SET);
//     assert(fread(int_buf_, sizeof(size_t), 1, f_pos_)==1);
//     size_t t = *int_buf_;
    
//     assert(fread(int_buf_+1, t*sizeof(size_t), 1, f_pos_)==1);
//     v2.attach((const size_t*)int_buf_);
    
//     return true;
//   }

  inline uint32_t next4Tg(str_vector& strs, int_vector& ints)
  {
    if (!str_in_mem())
      str_load_from(str_cache_pos_+str_start_addr_);

    strs.attach((const char*)(str_buf_+str_cache_pos_+sizeof(uint32_t)));
    
    uint32_t docid = *(uint32_t*)(str_buf_+str_cache_pos_);
    
    str_cache_pos_ += sizeof(uint32_t)+sizeof(size_t) + strs.size();

    if (!int_in_mem())
      int_load_from(int_cache_pos_*sizeof(size_t)+int_start_addr_);

    ints.attach(int_buf_+int_cache_pos_);
    int_cache_pos_ += *(int_buf_+int_cache_pos_)+1;

    return docid;
  }  

  inline size_t doc_num()const
  {
    return count_;
  }
  
}
  ;
#endif

NS_IZENELIB_AM_END

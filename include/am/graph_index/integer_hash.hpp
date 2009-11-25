/**
   @file integer_hash.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef INTEGER_HASH_HPP
#define INTEGER_HASH_HPP

#include<types.h>
#include <ostream>
#include <iostream>
#include "dyn_array.hpp"


/**
   @class IntegerHashTable
   @brief This is for hash fixed size data.
 */
NS_IZENELIB_AM_BEGIN

template<
  class VALUE_TYPE = uint32_t,//!< data type
  uint32_t ENTRY_SIZE = 100000,//!< size of entries
  bool BUCKET_SORTED = false//!< true if it needs to get every slot sorted. This will delay insertion but accelerate lookups.
  >
class IntegerHashTable
{
  typedef DynArray<VALUE_TYPE, BUCKET_SORTED> bucket_t;
  bucket_t* entry_[ENTRY_SIZE];//!< entry vector
  uint32_t num_;//!< data number

  /**
     @return the amount of entry that has been taken.
   */
  uint32_t entry_num_()const
  {
    uint32_t r = 0;

    for (uint32_t i=0; i<ENTRY_SIZE; ++i)
      if (entry_[i] != NULL)
        ++r;
    return r;
  }
  
public:
  IntegerHashTable()
  {
    for (uint32_t i=0; i<ENTRY_SIZE; ++i)
      entry_[i] = NULL;
    num_ = 0;
  }

  ~IntegerHashTable()
  {
    clear();
  }
  
  
  bool insert(const VALUE_TYPE& v)
  {
    uint32_t i = v% ENTRY_SIZE;

    if (entry_[i]==NULL)
    {
      entry_[i] = new bucket_t();
      entry_[i]->push_back(v);
      ++num_;
      return true;
    }

    if (entry_[i]->find(v) == bucket_t::NOT_FOUND)
    {
      entry_[i]->push_back(v);
      ++num_;
      return true;
    }
    return false;
    
  }
  
  bool find(VALUE_TYPE& v)const
  {
    uint32_t i = v% ENTRY_SIZE;

    if (entry_[i]==NULL)
      return false;

    typename bucket_t::size_t j = entry_[i]->find(v);
    
    if (j == bucket_t::NOT_FOUND)
      return false;

    v = entry_[i]->at(j);
    return true;
    
  }

  bool del(const VALUE_TYPE& v)
  {
    uint32_t i = v% ENTRY_SIZE;

    if (entry_[i] == NULL)
      return false;

    typename bucket_t::size_t j = entry_[i]->find(v);
    if (j == bucket_t::NOT_FOUND)
      return false;

    entry_[i]->erase(j);
    --num_;
    return true;
  }

  /**
     Updata v1 to v2
   **/
  inline bool update(const VALUE_TYPE& v1, const VALUE_TYPE& v2)
  {
    if (find(v2)!=bucket_t::NOT_FOUND || !del(v1))
      return false;

    return insert(v2);
  }

  inline uint32_t num()const
  {
    return num_;
  }

  /**
     @brief remove all the data and deallocate.
   */
  void clear()
  {
    for (uint32_t i=0; i<ENTRY_SIZE; ++i)
      if (entry_[i] != NULL)
      {
        delete entry_[i];
        entry_[i] = NULL;
      }

    num_ = 0;
  }

  /**
     @brief size of the entire table that is needed to save it.
   */
  uint32_t save_size()const
  {
    uint32_t s = 2*sizeof(uint32_t);
    
    for (uint32_t i=0; i<ENTRY_SIZE; ++i)
      if (entry_[i] != NULL)
      {
        s += sizeof(uint32_t);
        s += entry_[i]->save_size();
      }
    
    return s;
  }
  
  uint32_t save(FILE* f, uint64_t addr = -1)
  {
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);

    uint32_t s = 2*sizeof(uint32_t);
    
    IASSERT(fwrite(&num_, sizeof(uint32_t), 1, f)==1);
    uint32_t ety_num = entry_num_();
    IASSERT(fwrite(&ety_num, sizeof(uint32_t), 1, f)==1);
    
    for (uint32_t i=0; i<ENTRY_SIZE; ++i)
      if (entry_[i] != NULL)
      {
        IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
        s += sizeof(uint32_t);
        s += entry_[i]->save(f);
      }
    
    return s;
  }

  
  uint32_t load(FILE* f, uint64_t addr = -1)
  {
    clear();
    
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);

    uint32_t s = 2*sizeof(uint32_t);
    
    IASSERT(fread(&num_, sizeof(uint32_t), 1, f)==1);
    uint32_t ety_num = 0;
    IASSERT(fread(&ety_num, sizeof(uint32_t), 1, f)==1);
    
    for (uint32_t i=0; i<ety_num; ++i)
    {
      uint32_t j = 0;
      IASSERT(fread(&j, sizeof(uint32_t), 1, f)==1);
      s += sizeof(uint32_t);
      entry_[j] = new bucket_t();
      s += entry_[j]->load(f);
    }

    return s;
  }

}
  ;

NS_IZENELIB_AM_END
#endif

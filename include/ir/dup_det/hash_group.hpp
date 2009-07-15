#ifndef HASH_GROUP_HPP
#define HASH_GROUP_HPP

#include <types.h>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <ir/dup_det/prime_gen.hpp>

NS_IZENELIB_IR_BEGIN

template <
  uint32_t ENTRY_SIZE = 100000,
  class  UNIT_TYPE = uint64_t
  >
class HashGroup
{
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef Vector32::size_t size_t;
  typedef izenelib::ir::PrimeGen<> Prime;
  typedef HashGroup<ENTRY_SIZE, UNIT_TYPE> SelfT;
  
protected:
  Vector32 gids_;
  Vector32 doc_hash_;
  FILE* f_;
  uint32_t group_num_;

public:
  static Prime prime_;


public:
  inline HashGroup(const char* filenm)
  {
    size_t doc_num=0;
    gids_.reserve(ENTRY_SIZE);
    doc_hash_.reserve(doc_num);

    f_ = fopen(filenm, "r+");
    if (f_ == NULL)
    {
      for (uint32_t i=0; i<ENTRY_SIZE; i++)
        gids_.push_back(-1);
      f_ = fopen(filenm, "w");
      if (f_ == NULL)
      {
        std::cout<<"Can't create file: "<<filenm<<std::endl;
        return;
      }
    }
    else
    {
      load();
    }
    
    group_num_ = 0;
  }

  inline ~HashGroup()
  {
    flush();
    fclose(f_);
  }
  
  inline void load()
  {
    size_t doc_num = 0;
    fread(&doc_num, sizeof(size_t), 1,f_);
    fread(doc_hash_.array(doc_num), doc_hash_.size(), 1, f_);
    fread(gids_.array(ENTRY_SIZE), gids_.size(), 1, f_);
  }

  inline void flush()
  {
    size_t doc_num = doc_hash_.length();
    
    fwrite(&doc_num, sizeof(size_t), 1,f_);
    fwrite(doc_hash_.data(), doc_hash_.size(), 1, f_);
    fwrite(gids_.data(), gids_.size(), 1, f_);
    fflush(f_);
  }

  inline void insert(UNIT_TYPE fp)
  {
    size_t i = fp % ENTRY_SIZE;
    if (gids_.at(i)== (uint32_t)-1)
    {
      gids_[i] = prime_.next();
      ++group_num_;
    }
    
    doc_hash_.push_back(i);
  }

  inline void insert(UNIT_TYPE fp, uint32_t gid)
  {
    size_t i = fp % ENTRY_SIZE;
    if (gids_.at(i)== (uint32_t)-1)
    {
      gids_[i] = gid;
    }
    else
    if (gids_[i]%gid !=0 )
      gids_[i] *= gid;
    doc_hash_.push_back(i);
  }

  inline uint32_t operator[] (size_t i)const
  {
    return gids_.at(doc_hash_.at(i));
  }

  inline size_t length()const
  {
    return doc_hash_.length();
  }

  inline uint32_t group_num()const
  {
    return group_num_;
  }

    
  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const SelfT& v)
  {
    for (size_t i =0; i<v.length(); i++)
      os<<v[i]<<" ";

    return os;
  }

  
}
  ;

template <
  uint32_t ENTRY_SIZE,
  class  UNIT_TYPE
  >
izenelib::ir::PrimeGen<> HashGroup<ENTRY_SIZE, UNIT_TYPE>::prime_ =  izenelib::ir::PrimeGen<>("./prime_num.dat");

NS_IZENELIB_IR_END
#endif

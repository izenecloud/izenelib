/**
   @file hash_table.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <types.h>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <ir/dup_det/prime_gen.hpp>

NS_IZENELIB_IR_BEGIN

/**
   @class HashTable
   @brief used for pre-clustring, Those who share one same chunk in FP will be linked together.
 */
template <
  uint32_t ENTRY_SIZE = 1000000,
  class  UNIT_TYPE = uint64_t
  >
class HashTable
{
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<Vector32*> Vector32Ptr;
  typedef Vector32::size_t size_t;
  typedef HashTable<ENTRY_SIZE, UNIT_TYPE> SelfT;
  
protected:
  Vector32 gids_;//!< store index of entrance
  Vector32Ptr entry_;//!< table entrance
  FILE* f_;

public:
  inline HashTable(const char* filenm)
  {
    size_t doc_num=0;
    entry_.reserve(ENTRY_SIZE);
    for (uint32_t i=0; i<ENTRY_SIZE; i++)
        entry_.push_back(NULL);

    f_ = fopen(filenm, "r+");
    if (f_ == NULL)
    {
      f_ = fopen(filenm, "w+");
      if (f_ == NULL)
      {
        std::cout<<"Can't create file: "<<filenm<<std::endl;
        return;
      }
      fwrite(&doc_num, sizeof(size_t), 1,f_);
    }
    else
    {
      //std::cout<<"Loading....\n";
      load();
    }
  }

  inline ~HashTable()
  {
    fclose(f_);
    for (size_t i=0; i<entry_.length(); i++)
    {
      Vector32* v = entry_.at(i);
      if (v==NULL)
        continue;
      delete v;
    }
  }
  
  inline void load()
  {
    size_t doc_num = 0;
    fseek(f_, 0, SEEK_SET);
    fread(&doc_num, sizeof(size_t), 1,f_);
    
    IASSERT(fread(gids_.array(doc_num), doc_num*sizeof(uint32_t), 1, f_)==1);

    size_t index = 0;
    IASSERT(fread(&index, sizeof(size_t), 1, f_)==1);
    for (size_t i=0; i<entry_.length(); i++)
    {
      if (index != i)
        continue;

      size_t len = 0;
      IASSERT(fread(&len, sizeof(size_t), 1, f_)==1);
      entry_[i] = new Vector32();
      IASSERT(fread(entry_.at(i)->array(len), len*sizeof(uint32_t), 1, f_)==1);
      if (fread(&index, sizeof(size_t), 1, f_)!=1)
        index = -1;
    }
  }

  inline void flush()
  {
    size_t doc_num = gids_.length();

    fseek(f_, 0, SEEK_SET);
    fwrite(&doc_num, sizeof(size_t), 1,f_);
    
    fwrite(gids_.data(), gids_.size(), 1, f_);

    for (size_t i=0; i<entry_.length(); i++)
    {
      Vector32* v = entry_.at(i);
      if (v==NULL)
        continue;

      fwrite(&i, sizeof(size_t), 1, f_);
      size_t len = v->length();
      fwrite(&len, sizeof(size_t), 1, f_);
      fwrite(v->data(), v->size(), 1, f_);
    }
    
    fflush(f_);
  }

  inline void push_back(UNIT_TYPE fp)
  {
    size_t i = fp % ENTRY_SIZE;
    if (entry_.at(i) == NULL)
      entry_[i] = new Vector32();
    entry_.at(i)->push_back(gids_.length());
    gids_.push_back(i);
  }

  inline size_t length()const
  {
    return gids_.length();
  }


  inline void compact()
  {
    gids_.compact();
    for (size_t i=0; i<entry_.length(); i++)
    {
      Vector32* v = entry_.at(i);
      if (v==NULL)
        continue;
      v->compact();
    }
    
  }

  /**
     @return all the index of documents that share same chunk in FP.
   */
  inline const Vector32& operator[] (size_t i)
  {
    return *(entry_.at(gids_.at(i)));  
  }
  
  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const SelfT& v)
  {
    for (size_t i =0; i<v.length(); i++)
    {
      os<<v.gids_.at(i)<<"# ";
      Vector32* p = v.entry_.at(v.gids_.at(i));
      os<<(*p)<<std::endl;
    }

    return os;
  }

  
}
  ;

NS_IZENELIB_IR_END
#endif

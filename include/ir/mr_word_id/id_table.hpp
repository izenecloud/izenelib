/**
   @file id_table.hpp
   @author Kevin Hu
   @date 2010.01.08
*/
#ifndef ID_TABLE_HPP
#define ID_TABLE_HPP

#include <types.h>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include <util/hashFunction.h>
#include <am/graph_index/dyn_array.hpp>

NS_IZENELIB_IR_BEGIN
/**
 * @class IdTable
 * @brief A id to id hash table.
 *
 **/

template<
  typename IDType = uint32_t,
  uint8_t ENTRY_POW = 17
  >
class IdTable
{
  enum {ENTRY_SIZE = (2<<ENTRY_POW)};
  enum {ENTRY_MASK = ENTRY_SIZE-1};
  enum 
    {
      INIT_BUCKET_SIZE = 64
    };

  typedef izenelib::am::DynArray<IDType> bucket_t;
  
protected:
  bucket_t* entry_[ENTRY_SIZE];//!< entry of table
  uint32_t count_;

  void clean_()
  {
    for(uint32_t i=0; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]!= NULL)
        delete (entry_[i]);
      
      entry_[i] = NULL;
    }

    count_ = 0;
  }
  
public:
  /**
     @brief a constructor
  */
  IdTable()
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      entry_[i] = NULL;
    }
    count_ = 0;
  }
    
  /**
     @brief a destructor
  */
  ~IdTable()
  {
    clean_();
  }

  uint32_t num_items() const
  {
    return count_;
  }

  void display(std::ostream & os=std::cout)
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      bucket_t* pBkt = (bucket_t*)entry_[i];

      if (pBkt ==NULL)
        continue;

      os<<endl<<i<<" Entry===========\n";
      os<<(*pBkt)<<std::endl;
    }
  
  }

  bool insert(IDType id1, IDType id2)
  {
    if (find(id1) != (IDType)(-1))
      return false;

    uint32_t idx  = id1 & ENTRY_MASK;
    if (entry_[idx] == NULL)
      entry_[idx] = new bucket_t();

    entry_[idx]->push_back(id1);
    entry_[idx]->push_back(id2);
    ++count_;
    
    return true;
  }
  
  bool update(IDType id1, IDType id2)
  {
    if (find(id1) == (IDType)(-1))
      return false;
    
    uint32_t idx  = id1 & ENTRY_MASK;
    IASSERT (entry_[idx] != NULL);

    for (uint32_t i=0; i<entry_[idx]->length(); ++i)
      if (entry_[idx]->at(i++) == id1)
      {
        (*entry_[idx])[i] = id2;
        return true;
      }
    
    return false;
  }

  bool remove(IDType id)
  {
    uint32_t idx  = id & ENTRY_MASK;
    if (entry_[idx] == NULL)
      return false;
    
    for (uint32_t i=0; i<entry_[idx]->length(); i+=2)
      if (entry_[idx]->at(i) == id)
      {
        entry_[idx]->erase(i);
        entry_[idx]->erase(i);
        return true;
      }

    return false;
  }
  
  IDType find(IDType id1)const
  {
    uint32_t idx  = id1 & ENTRY_MASK;
    if (entry_[idx] == NULL)
      return (IDType)(-1);

    
    for (uint32_t i=0; i<entry_[idx]->length(); ++i)
      if (entry_[idx]->at(i++) == id1)
        return entry_[idx]->at(i);

    return (IDType)(-1);
  }
  
  void save(FILE* f)const
  {
    fseek(f, 0, SEEK_SET);
    IASSERT(fwrite(&count_, sizeof(uint32_t), 1, f)==1);
    
    uint32_t i=0;
    for(; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]==NULL)
        continue;

      IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
      entry_[i]->save(f);
    }

    i=-1;
    IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
  }
  
  void load(FILE* f)
  {
    clean_();
    
    uint32_t i=0;
    
    fseek(f, 0, SEEK_SET);
    IASSERT(fread(&count_, sizeof(uint32_t), 1, f)==1);

    IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    while (i != (uint32_t)-1)
    {
      entry_[i] = new bucket_t();
      entry_[i]->load(f);
      IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    }
  }
  
};

NS_IZENELIB_IR_END

#endif

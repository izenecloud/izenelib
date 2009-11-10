#ifndef ID_TRANSFER_HPP
#define ID_TRANSFER_HPP

#include<types.h>
#include <ostream>
#include <iostream>
#include "dyn_array.hpp"
#include "integer_hash.hpp"


NS_IZENELIB_AM_BEGIN

struct ID_64_32
{
  char id64[8];
  char id32[4];

  uint64_t& ID64_()
  {
    return *(uint64_t*)id64;
  }

  uint32_t& ID32_()
  {
    return *(uint32_t*)id32;
  }

  uint64_t ID64()const
  {
    return *(uint64_t*)id64;
  }

  uint32_t ID32()const
  {
    return *(uint32_t*)id32;
  }
  
  inline ID_64_32(uint64_t i, uint32_t j)
  {
    ID64_() = i;
    ID32_() = j;
  }
  
  inline ID_64_32(uint64_t i)
  {
    ID64_() = i;
    ID32_() = -1;
  }
  
  inline ID_64_32()
  {
    ID64_() = 0;
    ID32_() = 0;
  }
  
  inline ID_64_32(const ID_64_32& other)
  {
    ID64_() = other.ID64();
    ID32_() = other.ID32();
  }

  inline ID_64_32& operator = (const ID_64_32& other)
  {
    ID64_() = other.ID64();
    ID32_() = other.ID32();
    return *this;
  }

  inline bool operator == (const ID_64_32& other)const
  {
    return (ID64() == other.ID64());
  }

  inline bool operator != (const ID_64_32& other)const
  {
    return (ID64() != other.ID64());
  }

  inline bool operator < (const ID_64_32& other)const 
  {
    return (ID64() < other.ID64());
  }

  inline bool operator > (const ID_64_32& other)const
  {
    return (ID64() > other.ID64());
  }

  inline bool operator <= (const ID_64_32& other)const 
  {
    return (ID64() <= other.ID64());
  }

  inline bool operator >= (const ID_64_32& other)const
  {
    return (ID64() >= other.ID64());
  }

  inline uint32_t operator % (uint32_t e)const
  {
    return (ID64() % e);
  }

  
  
}
  ;


struct ID_32_64
{
  char id32[4];
  char id64[8];

  uint32_t& ID32_()
  {
    return *(uint32_t*)id32;
  }

  uint64_t& ID64_()
  {
    return  *(uint64_t*)id64;
  }

  uint32_t ID32()const
  {
    return *(uint32_t*)id32;
  }

  uint64_t ID64()const
  {
    return *(uint64_t*)id64;
  }
  
  inline ID_32_64(uint32_t i, uint64_t j)
  {
    ID32_() = i;
    ID64_() = j;
  }
  
  inline ID_32_64(uint32_t i)
  {
    ID32_() = i;
    ID64_() = 0;
  }
  
  inline ID_32_64()
  {
    ID32_() = 0;
    ID64_() = 0;
  }
  
  inline ID_32_64(const ID_32_64& other)
  {
    ID32_() = other.ID32();
    ID64_() = other.ID64();
  }

  inline ID_32_64& operator = (const ID_32_64& other)
  {
    ID32_() = other.ID32();
    ID64_() = other.ID64();
    return *this;
  }

  inline bool operator == (const ID_32_64& other)const
  {
    return (ID32() == other.ID32());
  }

  inline bool operator != (const ID_32_64& other)const
  {
    return (ID32() != other.ID32());
  }

  inline bool operator < (const ID_32_64& other)const 
  {
    return (ID32() < other.ID32());
  }

  inline bool operator > (const ID_32_64& other)const
  {
    return (ID32() > other.ID32());
  }

  inline bool operator <= (const ID_32_64& other)const 
  {
    return (ID32() <= other.ID32());
  }

  inline bool operator >= (const ID_32_64& other)const
  {
    return (ID32() >= other.ID32());
  }

  inline uint32_t operator % (uint32_t e)const
  {
    return (ID32() % e);
  }

  
  
}
  ;

template<
  uint32_t START_ID = 1,
  uint32_t ENTRY_SIZE = 100000
  >
class IdTransfer
{
  typedef IntegerHashTable<struct ID_64_32, ENTRY_SIZE> hash64_32_t;
  typedef IntegerHashTable<struct ID_32_64, ENTRY_SIZE> hash32_64_t;

  uint32_t id32_;
  hash64_32_t table64_32_;
  hash32_64_t table32_64_;
  

public:
  IdTransfer()
    :id32_(START_ID)
  {
  }

  uint32_t insert(uint64_t id64)
  {
    uint32_t r = get32(id64);
    if (r != (uint32_t)-1)
      return r;

    table64_32_.insert(ID_64_32(id64, id32_));

    r = id32_;
    IASSERT(table32_64_.insert(ID_32_64(id32_, id64)));
    ++id32_;

    return r;
  }

  uint64_t get64(uint32_t id32)const
  {
    ID_32_64 id(id32);

    if(!table32_64_.find(id))
      return -1;
    return id.ID64();
  }

  uint32_t get32(uint64_t id64)const
  {
    ID_64_32 id(id64);

    if(!table64_32_.find(id))
      return -1;
    return id.ID32();
  }

  uint32_t save(FILE* f, uint64_t addr = -1)
  {
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);
    
    uint32_t s = sizeof(uint32_t);

    IASSERT(fwrite(&id32_, sizeof(uint32_t), 1, f)==1);

    s += table32_64_.save(f);
    s += table64_32_.save(f);

    return s;
  }

  uint32_t load(FILE* f, uint64_t addr = -1)
  {
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);
    
    uint32_t s = sizeof(uint32_t);

    IASSERT(fread(&id32_, sizeof(uint32_t), 1, f)==1);

    s += table32_64_.load(f);
    s += table64_32_.load(f);

    return s;
  }
  
}
  ;

NS_IZENELIB_AM_END
#endif

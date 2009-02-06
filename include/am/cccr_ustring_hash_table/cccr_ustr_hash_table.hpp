#ifndef CCCR_STR_HASH_TABLE
#define CCCR_STR_HASH_TABLE

#include<string>
#include <iostream>
#include <types.h>
#include <util/log.h>
#include <stdio.h>
#include <ustring/UString.h>

using namespace sf1lib;
using namespace std;

#define PAGE_EXPANDING 1
#define EXACT_EXPANDING 0

class simple_hash
{
public:
  static uint32_t getValue(const string& key)
  {
    uint32_t convkey = 0;
    const char* str = (const char*)key.c_str();
    for (size_t i = 0; i < key.size(); i++)
      convkey = 37*convkey + *str++;

    return convkey;
  }
  
}
  ;

NS_IZENELIB_AM_BEGIN
/**
 *@brief CCCR_StrHashTable stands for Cache-Conscious Collision Resolution String Hash Table.
 *
 * This is based on work of Nikolas Askitis and Justin Zobel, 'Cache-Conscious Collision Resolution
 *     in String Hash Tables' .On typical current machines each cache miss incurs a delay of
 *hundreds of clock cycles while data is fetched from memory. This approach both
 *saves space and eliminates a potential cache miss at each node access, at little cost.
 *In experiments with large sets of strings drawn from real-world data, we show that,
 *in comparison to standard chaining, compact-chain hash tables can yield both space
 *savings and reductions in per-string access times.
 *
 *
 **/
template<
  size_t ENTRY_SIZE,
  class HASH_FUNCTION = simple_hash,
  int EXPAND = PAGE_EXPANDING
  >
class CCCR_StrHashTable
{
#define INIT_BUCKET_SIZE 64

  typedef CCCR_StrHashTable<ENTRY_SIZE, HASH_FUNCTION,PAGE_EXPANDING> SelfType;
  
public:
  CCCR_StrHashTable()
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      entry_[i] = NULL;
    }
    
  }

  ~CCCR_StrHashTable()
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]!=NULL) delete entry_[i];
      entry_[i] = NULL;
    }
  }

  bool insert(const UString& str, uint64_t value)
  {
    string tmp(str.c_str(), str.size());
    return insert(tmp, value);
  }
  
  bool insert(const string& str, uint64_t value)
  {
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt == NULL)
    {
      entry_[idx] = pBkt = new char[INIT_BUCKET_SIZE];
      *(uint32_t*)(pBkt) = 64;
      *((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);
    }
    else
    {
      uint32_t content_len = *((uint32_t*)pBkt+1);
      uint32_t p = sizeof(uint32_t)*2;;
      while (p < content_len)
      {
        size_t len = *((size_t*)(pBkt+p));

        if (len != str.length())
        {
          p += sizeof (size_t) + len + sizeof(uint64_t);
          continue;
        }
        
        p += sizeof (size_t);

        size_t j=0;
        for (; j<len; j++)
          if (str[j] != pBkt[p+j])
            break;

        if (j == len)
          return true;
        
        p += len + sizeof (uint64_t);
      }      
    }
    

    uint32_t bs =  *(uint32_t*)(pBkt);
    uint32_t content_len = *((uint32_t*)pBkt+1);

    if (bs-content_len<str.length()+sizeof(size_t)+sizeof(uint64_t))
    {
      bs += EXPAND==PAGE_EXPANDING? INIT_BUCKET_SIZE: str.length()+sizeof(size_t)+sizeof(uint64_t);
      //content_len += str.length()+sizeof(uint32_t);
      pBkt = new char[bs];
      memcpy(pBkt, entry_[idx],  *(uint32_t*)(entry_[idx]) );
      delete entry_[idx];
      entry_[idx] = pBkt;
    }

    *((size_t*)(pBkt+content_len)) = str.length();
    content_len += sizeof(size_t);
    memcpy(pBkt+content_len, str.c_str(), str.length());
    content_len += str.length();
    *((uint64_t*)(pBkt+content_len)) = value;
    content_len += sizeof(uint64_t);

    *(uint32_t*)(pBkt) = bs;
    *((uint32_t*)pBkt+1) = content_len;

    return true;
    //LDBG_<<str;
    
  }

  
  uint64_t find(const UString& str)
  {
    string tmp(str.c_str(), str.size());
    return find(tmp);
  }

  uint64_t find(const string & str)
  {
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt == NULL)
      return -1;

    
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;;
    while (p < content_len)
    {
      size_t len = *((size_t*)(pBkt+p));
      p += sizeof (size_t);
      if (str.length() != len)
      {
        p += len + sizeof (uint64_t);
        continue;
      }

      size_t i = 0;
      for (; i<len; i++)
      {
        if (pBkt[p+i]!=str[i])
          break;
      }
      if (i != len)
      {
        p += len + sizeof (uint64_t);
        continue;
      }

      p += len;
      
      return *((uint64_t*)(pBkt+p));
      
    }

    return (uint64_t) -1;
  }

  
  bool del(const UString& str)
  {
    string tmp(str.c_str(), str.size());
    return del(tmp);
  }

  bool del(const string& str)
  {
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt ==NULL)
      return -1;

    uint32_t bs =  *(uint32_t*)(pBkt);
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;;
    while (p < content_len)
    {
      size_t len = *((size_t*)(pBkt+p));
      p += sizeof (size_t);
      if (str.length() != len)
      {
        p += len + sizeof (uint64_t);
        continue;
      }

      size_t i = 0;
      for (; i<len; i++)
      {
        if (pBkt[p+i]!=str[i])
          break;
      }
      if (i != len)
      {
        p += len + sizeof (uint64_t);
        continue;
      }

      p += len;
      
      *((uint64_t*)(pBkt+p)) = (uint64_t)-1;
      
      return true;
    }

    return false;
  }
  
  bool update(const UString& str, uint64_t value)
  {
    string tmp(str.c_str(), str.size());
    return update(tmp, value);
  }

  bool update(const string& str, uint64_t value)
  {
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt ==NULL)
      return -1;

    uint32_t bs =  *(uint32_t*)(pBkt);
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;;
    while (p < content_len)
    {
      size_t len = *((size_t*)(pBkt+p));
      p += sizeof (size_t);
      if (str.length() != len)
      {
        p += len + sizeof (uint64_t);
        continue;
      }

      size_t i = 0;
      
      for (; i<len; i++)
      {
        if (pBkt[p+i]!=str[i])
          break;
      }
      if (i != len)
      {
        p += len + sizeof (uint64_t);
        continue;
      }

      p += len;
      
      *((uint64_t*)(pBkt+p)) = value;
      
      return true;
    }

    return false;
  }
  
friend ostream& operator << ( ostream& os, const SelfType& node)  
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      char* pBkt = node.entry_[i];
    
      if (pBkt ==NULL)
        continue;

      os<<endl<<i<<" Entry===========\n";
    
      uint32_t bs =  *(uint32_t*)(pBkt);
      uint32_t content_len = *((uint32_t*)(pBkt)+1);
      os<<"bucket size: "<<bs<<endl;
      os<<"content len: "<<content_len<<endl;
    
      uint32_t p = sizeof(uint32_t)*2;;
      while (p < content_len)
      {
        size_t len = *((size_t*)(pBkt+p));
        p += sizeof (size_t);
        os<<"("<<len<<")";

        for (size_t j=0; j<len; j++)
          os<<pBkt[p+j];
        p += len;
        
        os<<"=>"<<*((uint64_t*)(pBkt+p))<<endl;
        p += sizeof (uint64_t);
      }      
    }

    return os;
  }

  bool unload(FILE* f)
  {
    fseek(f, 0, SEEK_SET);
    
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      char* pBkt = entry_[i];
    
      if (pBkt ==NULL)
        continue;

      if ( fwrite(&i, sizeof(size_t), 1, f)!=1)
        return false;
      
      uint32_t bs =  *(uint32_t*)(pBkt);
      if ( fwrite(pBkt, bs, 1, f)!=1)
        return false;
      
    }

    return false;
  }

  bool load(FILE* f)
  {
    fseek(f, 0, SEEK_SET);
    size_t s;
    
    if(fread(&s, sizeof(size_t), 1, f)!=1)
      return false;
    
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      if (i==s)
      {
        uint32_t bs;
        if(fread(&bs, sizeof(uint32_t), 1, f)!=1)
          return false;

        entry_[i] = new char[bs];
        char* pBkt = entry_[i];
        *(uint32_t*)pBkt = bs;
        if(fread(pBkt+sizeof(uint32_t),bs-sizeof(uint32_t), 1, f)!=1)
          return false;

        if (feof(f))
          return true;
        
        if(fread(&s, sizeof(size_t), 1, f)!=1)
          return false;
      }
    }
    return true;
  }
  
  
  
protected:
  char* entry_[ENTRY_SIZE];
}
  ;


NS_IZENELIB_AM_END

#endif

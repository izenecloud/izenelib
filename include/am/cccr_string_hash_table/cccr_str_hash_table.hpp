#ifndef CCCR_STR_HASH_TABLE
#define CCCR_STR_HASH_TABLE

#include<string>
#include <iostream>
#include <types.h>
#include <util/log.h>
#include <cstdio>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <am/am.h>

#define PAGE_EXPANDING 1
#define EXACT_EXPANDING 0

using namespace std;

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

class numeric_hash
{
public:
  static uint32_t getValue(const unsigned int& key)
  {
    uint32_t k = 12;
    uint32_t u = 99;
    uint32_t p = 1000000;

    return (k*key+u)%p;
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
//////////////////////////////// This is for numeric key /////////////////////////////////
template<
  class KeyType = string,
  class ValueType = uint64_t,
  size_t ENTRY_SIZE = 131072,
  class HASH_FUNCTION = simple_hash,
  int EXPAND = PAGE_EXPANDING
  >
class CCCR_StrHashTable :public AccessMethod<KeyType, ValueType>
{
#define INIT_BUCKET_SIZE 64

  typedef CCCR_StrHashTable<KeyType, ValueType,ENTRY_SIZE, HASH_FUNCTION,EXPAND> SelfType;

  typedef boost::archive::binary_iarchive iarchive;
  typedef boost::archive::binary_oarchive oarchive;
  
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

  
  //bool insert(const KeyType& key, const ValueType& v)
  using AccessMethod<KeyType, ValueType>::insert;
  virtual bool insert(const DataType<KeyType,ValueType>& data)
  {
    KeyType key = data.key;
    ValueType v = data.value;
    
    uint64_t value = dataVec_.size();
    dataVec_.push_back(v);
    
    uint32_t idx = HASH_FUNCTION::getValue(key) % ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt == NULL)
    {
      entry_[idx] = pBkt = new char[INIT_BUCKET_SIZE];
      *(uint32_t*)(pBkt) = 64;//bucket size
      *((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);//how much size of bucket has been taken?
    }
    else
    {//does it exist?
      uint32_t content_len = *((uint32_t*)pBkt+1);
      uint32_t p = sizeof(uint32_t)*2;;
      while (p < content_len)
      {
        if (*((KeyType*)(pBkt+p)) == key)
        {
          p += sizeof(KeyType);
          *(uint64_t*)(pBkt+p) = value;
          return true;
        }

        p += sizeof(KeyType) + sizeof(uint64_t);
      }      
    }
    

    uint32_t bs =  *(uint32_t*)(pBkt);
    uint32_t content_len = *((uint32_t*)pBkt+1);

    if (bs-content_len<sizeof(KeyType)+sizeof(uint64_t))
    {
      bs += EXPAND==PAGE_EXPANDING? INIT_BUCKET_SIZE: sizeof(KeyType)+sizeof(uint64_t);
      //content_len += str.length()+sizeof(uint32_t);
      pBkt = new char[bs];
      memcpy(pBkt, entry_[idx],  *(uint32_t*)(entry_[idx]) );
      delete entry_[idx];
      entry_[idx] = pBkt;
    }

    *((KeyType*)(pBkt+content_len)) = key;
    content_len += sizeof(KeyType);
    

    *((uint64_t*)(pBkt+content_len)) = value;
    content_len += sizeof(uint64_t);

    *(uint32_t*)(pBkt) = bs;
    *((uint32_t*)pBkt+1) = content_len;

    count_++;
    return true;
    //LDBG_<<str;

  }


  int num_items()
  {
    return count_;
  }
  
  ValueType* find(const KeyType & key)
  {
    uint32_t idx = HASH_FUNCTION::getValue(key) %ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt == NULL)
      return NULL;

    
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;;
    while (p < content_len)
    {
      if (*((KeyType*)(pBkt+p)) == key)
      {
        p += sizeof(KeyType);
        if(*(uint64_t*)(pBkt+p)!=(uint64_t)-1)
          return &dataVec_[*(uint64_t*)(pBkt+p)];
        else
          return NULL;
      }
      p += sizeof (KeyType) + sizeof (uint64_t);      
    }

    return NULL;
  }

  bool del(const KeyType& key)
  {
    uint32_t idx = HASH_FUNCTION::getValue(key)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt ==NULL)
      return -1;
    
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;;
    while (p < content_len)
    {
      
      if (*((KeyType*)(pBkt+p)) == key)
      {
        p += sizeof(KeyType);
        *(uint64_t*)(pBkt+p)=(uint64_t)-1;
        count_--;
        return true;
      }

      p += sizeof (KeyType) + sizeof (uint64_t);
      
    }

    return false;
  }

  //bool update(const KeyType& key, const ValueType& v)
  using AccessMethod<KeyType, ValueType>::update;
  virtual bool update(const DataType<KeyType,ValueType>& data)
  {
    KeyType key = data.key;
    ValueType v = data.value;
    
    uint64_t value = dataVec_.size();
    dataVec_.push_back(v);

    uint32_t idx = HASH_FUNCTION::getValue(key)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt ==NULL)
      return false;
    
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;;
    while (p < content_len)
    {
      if (*((KeyType*)(pBkt+p)) == key)
      {
        p += sizeof(KeyType);
        if(*(uint64_t*)(pBkt+p)!=(uint64_t)-1)
        {
          *(uint64_t*)(pBkt+p) = value;
          return true;
        }
        else
          return false;
      }

      p += sizeof (KeyType) + sizeof(uint64_t);
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
        os<<"[";
        os << *((size_t*)(pBkt+p));
        os<<",";
        
        p += sizeof (KeyType);
        os << *((uint64_t*)(pBkt+p));
        os<<"]";

        p += sizeof (uint64_t);
      }      
    }

    return os;
  }


  bool save(const string& keyFilename, const string& valueFilename)
  {
    FILE* f = fopen(keyFilename.c_str(), "w+");
    if (f ==NULL)
    {
      cout<<"\nCan't open file: "<<keyFilename<<endl;
      return false;
    }

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


    fclose(f);
    
    ofstream of(valueFilename.c_str());
    oarchive oa(of);
    size_t size = dataVec_.size();
    oa << size;
    
    for(typename vector<ValueType>::iterator i =dataVec_.begin(); i!=dataVec_.end(); i++)
    {
      oa<<(*i);
    }

    of.close();

    return true;
  }

  bool load(const string& keyFilename, const string& valueFilename)
  {
    FILE* f = fopen(keyFilename.c_str(), "r");
    if (f ==NULL)
    {
      cout<<"\nCan't open file: "<<keyFilename<<endl;
      return false;
    }
    
    fseek(f, 0, SEEK_SET);
    size_t s;
    
    while(fread(&s, sizeof(size_t), 1, f)==1)
    {
      uint32_t bs;
      if(fread(&bs, sizeof(uint32_t), 1, f)!=1)
        return false;

      entry_[s] = new char[bs];
      char* pBkt = entry_[s];
      *(uint32_t*)pBkt = bs;
      if(fread(pBkt+sizeof(uint32_t),bs-sizeof(uint32_t), 1, f)!=1)
        return false;
    }

    fclose(f);
    
    dataVec_.clear();
    
    ifstream ifs(valueFilename.c_str());
    iarchive ia(ifs);
    size_t size;
    ia>>size;

    for (size_t i =0; i<size; i++)
    {
      ValueType v;
      ia>>v;
      //cout<<v;
      dataVec_.push_back(v);
    }
        
    ifs.close();
    
    return true;
  }
  
    
protected:
  char* entry_[ENTRY_SIZE];
  vector<ValueType> dataVec_;
  int count_;
}
  ;

////////////////////////////////// For std::string key ///////////////////////////////////////////
template<
  class ValueType ,
  class HASH_FUNCTION,
  size_t ENTRY_SIZE,
  int EXPAND
  >
class CCCR_StrHashTable<string, ValueType,ENTRY_SIZE, HASH_FUNCTION, EXPAND> :public AccessMethod<string, ValueType>
{
#define INIT_BUCKET_SIZE 64

  typedef CCCR_StrHashTable<string, ValueType, ENTRY_SIZE, HASH_FUNCTION,EXPAND> SelfType;

  typedef boost::archive::binary_iarchive iarchive;
  typedef boost::archive::binary_oarchive oarchive;
public:
  CCCR_StrHashTable()
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      entry_[i] = NULL;
    }

    count_ = 0;
  }

  ~CCCR_StrHashTable()
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]!=NULL) delete entry_[i];
      entry_[i] = NULL;
    }
  }

  using AccessMethod<string, ValueType>::insert;
  virtual bool insert(const DataType<string,ValueType>& data)
  {
    string str = data.key;
    ValueType v = data.value;

    uint64_t value = dataVec_.size();
    dataVec_.push_back(v);
    
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt == NULL)
    {
      entry_[idx] = pBkt = new char[INIT_BUCKET_SIZE];
      *(uint32_t*)(pBkt) = 64;
      *((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);
    }
    else
    {//does it exist?
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
        {
          *(uint64_t*)(pBkt+p+j) = value;
          return true;
        }
        
        
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

    count_++;
    return true;
    //LDBG_<<str;
    
  }

  ValueType* find(const string & str)
  {
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt == NULL)
      return NULL;

    
    uint32_t content_len = *((uint32_t*)(pBkt)+1);
    
    uint32_t p = sizeof(uint32_t)*2;
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

      if (*((uint64_t*)(pBkt+p))!=(uint64_t)-1)
        return &(dataVec_[*((uint64_t*)(pBkt+p))]);
      else
        return NULL;    
    }

    return NULL;
    
  }

  int num_items() const 
  {
    return count_;
  }
  
  bool del(const string& str)
  {
    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt ==NULL)
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
      
      *((uint64_t*)(pBkt+p)) = (uint64_t)-1;

      if (count_>0)
        count_ --;
      
      return true;
    }

    return false;
  }

  using AccessMethod<string, ValueType>::update;
  
  virtual bool update(const DataType<string,ValueType>& data)
  {
    string str = data.key;
    ValueType v = data.value;

    uint32_t idx = HASH_FUNCTION::getValue(str)%ENTRY_SIZE;
    char* pBkt = entry_[idx];
    
    if (pBkt ==NULL)
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

      if (*((uint64_t*)(pBkt+p)) != (uint64_t)-1)
      {
        dataVec_[*((uint64_t*)(pBkt+p))] = v;
        return true;
      }
      else
        return false;
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


  bool save(const string& keyFilename, const string& valueFilename)
  {
    FILE* f = fopen(keyFilename.c_str(), "w+");
    if (f ==NULL)
    {
      cout<<"\nCan't open file: "<<keyFilename<<endl;
      return false;
    }

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


    fclose(f);
    
    ofstream of(valueFilename.c_str());
    oarchive oa(of);
    
    size_t size = dataVec_.size();
    oa << size;
    
    for(typename vector<ValueType>::iterator i =dataVec_.begin(); i!=dataVec_.end(); i++)
    {
      oa<<(*i);
    }

    of.close();

    return true;
  }

  bool load(const string& keyFilename, const string& valueFilename)
  {
    FILE* f = fopen(keyFilename.c_str(), "r");
    if (f ==NULL)
    {
      cout<<"\nCan't open file: "<<keyFilename<<endl;
      return false;
    }
    
    fseek(f, 0, SEEK_SET);
    size_t s;
    
    while(fread(&s, sizeof(size_t), 1, f)==1)
    {      
      uint32_t bs;
      if(fread(&bs, sizeof(uint32_t), 1, f)!=1)
        return false;
      
      entry_[s] = new char[bs];
      char* pBkt = entry_ [s];
      *(uint32_t*)pBkt = bs;
      if(fread(pBkt+sizeof(uint32_t),bs-sizeof(uint32_t), 1, f)!=1)
        return false;
      
    }

    fclose(f);

    dataVec_.clear();
    
    ifstream ifs(valueFilename.c_str());
    iarchive ia(ifs);
    
    size_t size;
    ia>>size;
    //dataVec_.resize(size);
    
    for (size_t i =0; i<size; i++)
    {
      ValueType v;
      ia>>v;
      dataVec_.push_back(v);
    }
        
    ifs.close();
    
    return true;
  }
  
  
  
protected:
  char* entry_[ENTRY_SIZE];
  vector<ValueType> dataVec_;
  int count_;
}
  ;


NS_IZENELIB_AM_END

#endif

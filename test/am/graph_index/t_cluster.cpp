#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ostream>

#include <am/graph_index/dyn_array.hpp>
#include <math.h>

using namespace izenelib::am;

/** 
 * @brief hash 32 bit int
 * 
 * @param key source
 * 
 * @return hashed value
 */
inline uint32_t hash32shift(uint32_t key) {
  key = ~key + (key << 15); // key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057; // key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);
  return key;
}

/** 
 * @brief hash unsinged 32-bit int
 * 
 * @param a source 
 * 
 * @return hashed value
 */
inline uint32_t hash( uint32_t a) {
  a = (a+0x7ed55d16) + (a<<12);
  a = (a^0xc761c23c) ^ (a>>19);
  a = (a+0x165667b1) + (a<<5);
  a = (a+0xd3a2646c) ^ (a<<9);
  a = (a+0xfd7046c5) + (a<<3);
  a = (a^0xb55a4f09) ^ (a>>16);
  return a;
}

/** 
 * @brief hash 32 bit
 * 
 * @param key source
 * 
 * @return hashed value
 */
inline uint32_t hash32shiftmult(uint32_t key) {
  uint32_t c2=0x27d4eb2d; // a prime or an odd constant
  key = (key ^ 61) ^ (key >> 16);
  key = key + (key << 3);
  key = key ^ (key >> 4);
  key = key * c2;
  key = key ^ (key >> 15);
  return key;
}

struct FP
{
  uint64_t fp;
  uint32_t index;

  inline FP()
  {
    fp = 0;
    index = -1;
  }
  
  explicit inline FP(uint64_t _fp, uint32_t _index)
  {
    fp = _fp;
    index = _index;
  }
  
  inline bool operator > (const struct FP& other)const
  {
    return fp>other.fp;
  }
  
  inline bool operator < (const struct FP& other)const
  {
    return fp<other.fp;
  }
    
  inline bool operator <= (const struct FP& other)const
  {
    return fp<=other.fp;
  }
  
  inline bool operator >= (const struct FP& other)const
  {
    return fp >= other.fp;
  }

  friend std::ostream& operator << (std::ostream& os, const FP& other)
  {
    os<<"<"<<other.index<<","<<other.fp<<">"<<std::endl;
    return os;
  }
}
  ;

void caculate_fp(uint32_t demen = 128,  const std::string& nm="./input")
{
  typedef DynArray<uint32_t> array_t;
  typedef DynArray<int> array_int_t;
  FILE* f = fopen(nm.c_str(), "r");
  uint32_t total = 0;
  IASSERT(fread(&total, sizeof(uint32_t), 1, f)==1);
  int fva[] = {-1, 1};

  array_int_t last;
  for (uint32_t j=0; j<demen; ++j)
      last.push_back(1);
  
  for (uint32_t i=0; i<total; ++i)
  {
    array_t arr;
    arr.load(f);
    array_int_t demens;
    demens.reserve(demen);
    for (uint32_t j=0; j<demen; ++j)
      demens.push_back(0);
    
    for (uint32_t j=0; j<arr.length(); ++j)
    {
      int n = demen-1;
      uint32_t key = arr.at(j);
      while(n >= 0) {
        uint32_t tkey = key;
        for (int p = 0; p < 32 && n >= 0; ++p) {
          demens[n] += fva[tkey & 0x01];
          tkey >>= 1;
          --n;
        }
        if(n >= 0) {
          key = hash32shift(key);
        }
      }
      
    }

    uint32_t dif = 0;
    for (uint32_t j=0; j<demen; ++j)
      if (demens.at(j)*last.at(j)<0)
        ++dif;

    last = demens;
    std::cout<<dif<<std::endl;
      
  }
}

void prime_gen(uint32_t s, std::vector<uint32_t>& v)
{
  for (uint32_t i = 3; v.size()<s; ++i)
  {
    uint32_t k = (uint32_t)sqrt(i);
    if (k*k == i)
      continue;

    uint32_t j=3;
    for (; j<k; ++j)
      if (i%j==0)
        break;

    if (j == k)
    {
      v.push_back(i);
      //std::cout<<i<<std::endl;
      //i is prime number
    }
  }
}

int main (int argc,char **argv)
{

  std::vector<uint32_t> v;
  prime_gen(20, v);
  uint64_t k=1;
  for (uint32_t i=0; i<20; ++i)
    k *= v[i];
  
  std::cout<<(uint64_t)-1<<"  "<<k<<std::endl;
  //DynArray<struct FP> fps;
  //caculate_fp();

}

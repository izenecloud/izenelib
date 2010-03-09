/**
   @file ter_freq.hpp
   @author Kevin Hu
   @date 2010.01.08
*/
#ifndef TERM_FREQ_HPP
#define TERM_FREQ_HPP

#include <types.h>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include <util/hashFunction.h>

NS_IZENELIB_IR_BEGIN
/**
 * @class TermFrequency
 * @brief TermFrequency is used for caculating the frequency of terms. It's a string hash table essentially.
 *
 **/

template<
  typename FREQ_TYPE = uint32_t,
  typename LenType = uint8_t,
  uint8_t ENTRY_POW = 17
  >
class TermFrequency
{
  enum {ENTRY_SIZE = (2<<ENTRY_POW)};
  enum {ENTRY_MASK = ENTRY_SIZE-1};
  enum 
    {
      INIT_BUCKET_SIZE = 64
    };
    

protected:
  char** entry_;//!< entry of table
  uint32_t count_;
  uint32_t bkt_iter_;
  uint32_t iter_;

  void clean_()
  {
    for(uint32_t i=0; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]!= NULL)
        free (entry_[i]);
      
      entry_[i] = NULL;
    }

    count_ = 0;
  }
  
public:
  /**
     @brief a constructor
  */
  TermFrequency()
  {
    entry_ = new char*[ENTRY_SIZE];
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      entry_[i] = NULL;
    }
    count_ = 0;
  }
    
  /**
     @brief a destructor
  */
  ~TermFrequency()
  {
    clean_();
    delete entry_;
  }

  void reset()
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
      char* pBkt = entry_[i];

      if (pBkt ==NULL)
        continue;

      os<<endl<<i<<" Entry===========\n";

      uint32_t bs = *(uint32_t*)(pBkt);
      os<<"bucket size: "<<bs<<endl;
      os<<"content size: "<<*((uint32_t*)(pBkt)+1)<<endl;

      uint32_t p = sizeof(uint32_t)*2;
      uint32_t c = *((uint32_t*)(pBkt)+1);
            
      while (p < c)
      {
        LenType len = *((LenType*)(pBkt+p));
        p += sizeof (LenType);
        os<<"("<<(uint32_t)len<<")";

        os<<(pBkt+p);
        p += len;

        os<<"=>"<<*((FREQ_TYPE*)(pBkt+p))<<std::endl;
        p += sizeof (FREQ_TYPE);
      }
    }
  
  }

  bool increase(const std::string& term)
  {
    if (term.length()==0)
      return false;

    uint32_t content_len = 0;
    uint32_t bs = 0;
    
    uint32_t idx = izenelib::util::HashFunction<std::string>::generateHash32(term) & ENTRY_MASK;

    char* pBkt = entry_[idx];

    if (pBkt == NULL)
    {
      entry_[idx] = pBkt = (char*)malloc(INIT_BUCKET_SIZE);
      bs = *(uint32_t*)(pBkt) = INIT_BUCKET_SIZE;
      content_len = *((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);
    }
    else
    {//does it exist?
      bs = *(uint32_t*)(pBkt);
      content_len = *((uint32_t*)pBkt+1);
      uint32_t p = sizeof(uint32_t)*2;;
      while (p < content_len)
      {
        LenType len = *((LenType*)(pBkt+p));
        p += sizeof (LenType);

        if (len-1 != (LenType)term.length())
        {
          p += len + sizeof (FREQ_TYPE);
          continue;
        }

        LenType j=0;
        for (; j<len-1; j++)
          if (term[j] != pBkt[p+j])
            break;

        if (j++ == len-1)
        {
          ++(*(FREQ_TYPE*)(pBkt+p+j));
          return true;
        }

        p += len + sizeof (FREQ_TYPE);
      }
    }

    IASSERT(bs>=content_len);
    
    while(bs-content_len<term.length()+1+sizeof(LenType)+sizeof(FREQ_TYPE))
    {
      bs += INIT_BUCKET_SIZE;

      if (bs-content_len<term.length()+1+sizeof(LenType)+sizeof(FREQ_TYPE))
        continue;
      //content_len += str.length()+sizeof(uint32_t);
      pBkt = (char*)realloc(pBkt, bs);
      entry_[idx] = pBkt;
    }

    if (term.length()+1>=(LenType)-1)
      std::cout<<"[ERROR] in TermFrequency increase(): Term length too long! =>"<<term<<std::endl;
    
    *((LenType*)(pBkt+content_len)) = term.length()+1;
    content_len += sizeof(LenType);
    memcpy(pBkt+content_len, term.c_str(), term.length());
    content_len += term.length();
    *(pBkt+content_len) = '\0';
    ++content_len; 
        
    *((uint32_t*)(pBkt+content_len)) = 1;
    content_len += sizeof(FREQ_TYPE);

    *(uint32_t*)(pBkt) = bs;
    *((uint32_t*)pBkt+1) = content_len;

    count_++;
    return false;
  }

  bool insert(const std::string& term, FREQ_TYPE freq)
  {
    if (term.length()==0)
      return false;

    if (find(term)>0)
      return false;
    
    uint32_t content_len = 0;
    uint32_t bs = 0;
    
    uint32_t idx = izenelib::util::HashFunction<std::string>::generateHash32(term) & ENTRY_MASK;

    char* pBkt = entry_[idx];

    if (pBkt == NULL)
    {
      entry_[idx] = pBkt = (char*)malloc(INIT_BUCKET_SIZE);
      bs = *(uint32_t*)(pBkt) = INIT_BUCKET_SIZE;
      content_len = *((uint32_t*)(pBkt)+1) = 2*sizeof(uint32_t);
    }
    else
    {//does it exist?
      bs = *(uint32_t*)(pBkt);
      content_len = *((uint32_t*)pBkt+1);
    }
    
    IASSERT(bs>=content_len);
    
    while(bs-content_len<term.length()+1+sizeof(LenType)+sizeof(FREQ_TYPE))
    {
      bs += INIT_BUCKET_SIZE;

      if (bs-content_len<term.length()+1+sizeof(LenType)+sizeof(FREQ_TYPE))
        continue;
      //content_len += str.length()+sizeof(uint32_t);
      pBkt = (char*)realloc(pBkt, bs);
      entry_[idx] = pBkt;
    }

    if (term.length()+1>=(LenType)-1)
      std::cout<<"[ERROR] in TermFrequency insert(): Term length too long! =>"<<term<<std::endl;
    
    *((LenType*)(pBkt+content_len)) = term.length()+1;
    content_len += sizeof(LenType);
    memcpy(pBkt+content_len, term.c_str(), term.length());
    content_len += term.length();
    *(pBkt+content_len) = '\0';
    ++content_len; 
        
    *((FREQ_TYPE*)(pBkt+content_len)) = freq;
    content_len += sizeof(FREQ_TYPE);

    *(uint32_t*)(pBkt) = bs;
    *((uint32_t*)pBkt+1) = content_len;

    count_++;
    return false;
  }

  FREQ_TYPE find(const std::string& term)const
  {
    if (term.length()==0)
      return 0;
    
    uint32_t idx = izenelib::util::HashFunction<std::string>::generateHash32(term) & ENTRY_MASK;

    char* pBkt = entry_[idx];

    if (pBkt == NULL)
      return 0;
    
    uint32_t  bs = *(uint32_t*)(pBkt);
    uint32_t  content_len = *((uint32_t*)pBkt+1);

    IASSERT(bs>=content_len);
    uint32_t p = sizeof(uint32_t)*2;;

    while (p < content_len)
    {
      LenType len = *((LenType*)(pBkt+p));
      p += sizeof (LenType);

      if (len-1 != (LenType)term.length())
      {
        p += len + sizeof (FREQ_TYPE);
        continue;
      }

      LenType j=0;
      for (; j<len-1; j++)
        if (term[j] != pBkt[p+j])
          break;

      if (j++ == len-1)
        return (*(FREQ_TYPE*)(pBkt+p+j));

      p += (uint32_t)len + sizeof (FREQ_TYPE);
    }

    return 0;
  }

  void save(FILE* f)const
  {
    char* pBkt = NULL;
    fseek(f, 0, SEEK_SET);
    IASSERT(fwrite(&count_, sizeof(uint32_t), 1, f)==1);
    
    uint32_t i=0;
    for(; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]==NULL)
        continue;

      IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
      pBkt = entry_[i];
      *(uint32_t*)(pBkt) = *((uint32_t*)pBkt+1);
      IASSERT(fwrite(pBkt, *(uint32_t*)(pBkt), 1, f)==1);
    }

    i=-1;

    IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
  }
  
  void load(FILE* f)
  {
    clean_();
    
    char* pBkt = NULL;
    uint32_t i=0;
    uint32_t bs = 0;
    
    fseek(f, 0, SEEK_SET);
    IASSERT(fread(&count_, sizeof(uint32_t), 1, f)==1);

    IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    while (i != (uint32_t)-1)
    {
      IASSERT(fread(&bs, sizeof(uint32_t), 1, f)==1);
      pBkt = entry_[i] = (char*)malloc(bs);
      *(uint32_t*)(pBkt) = bs;
      IASSERT(fread(pBkt+sizeof(uint32_t), bs-sizeof(uint32_t), 1, f)==1);
      
      IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    }
  }

  bool begin()
  {
    for(uint32_t i=0; i<ENTRY_SIZE; i++)
      if (entry_[i]!= NULL)
      {
        bkt_iter_ = i;
        iter_ = sizeof(uint32_t)*2;
        return true;
      }

    bkt_iter_ = -1;
    iter_ = -1;
    return false;
  }

  bool next(std::string& term, FREQ_TYPE& freq)
  {
    if (bkt_iter_ == (uint32_t)-1)
      return false;

    char* pBkt = entry_[bkt_iter_];
    if (pBkt == NULL)
      return false;

    
    term = std::string(pBkt+iter_+sizeof(LenType));
    iter_ += sizeof(LenType) + term.length()+1;
    freq = *(FREQ_TYPE*)(pBkt+iter_);
    iter_ += sizeof(FREQ_TYPE);

    if (iter_ < *((uint32_t*)pBkt+1))
      return true;

    for (uint32_t i=bkt_iter_+1; i<ENTRY_SIZE; ++i)
      if (entry_[i]!=NULL)
      {
        bkt_iter_ = i;
        iter_ = 2*sizeof (uint32_t);
        return true;
      }

    bkt_iter_ = -1;
    return false;
  }
  
};

NS_IZENELIB_IR_END

#endif

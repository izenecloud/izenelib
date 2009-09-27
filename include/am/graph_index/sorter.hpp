#ifndef SORTER_HPP
#define SORTER_HPP

#include<fstream>
#include<types.h>
#include <am/external_sort/alpha_sort.hpp>
#include <am/graph_index/dyn_array.hpp>
#include "addr_bucket.hpp"
#include<string>
#include<sstream>
#include <boost/thread/thread.hpp>
#include  <boost/bind.hpp>

NS_IZENELIB_AM_BEGIN


struct ADDR_STRUCT
{
  char integer[4];
  char addr[8];
  char len[2];

  uint32_t& INTEGER_()
  {
    return *(uint32_t*)integer;
  }

  uint64_t& ADDR_()
  {
    return *(uint64_t*)addr;
  }

  uint16_t& LEN_()
  {
    return *(uint16_t*)len;
  }

  uint32_t INTEGER()const
  {
    return *(uint32_t*)integer;
  }

  uint64_t ADDR()const
  {
    return *(uint64_t*)addr;
  }

  uint16_t LEN()const
  {
    return *(uint16_t*)len;
  }
  
  inline ADDR_STRUCT(uint32_t i, uint64_t j, uint16_t k)
  {
    INTEGER_() = i;
    ADDR_() = j;
    LEN_() = k;
  }
  
  inline ADDR_STRUCT(uint32_t i)
  {
    INTEGER_() = i;
    ADDR_() = 0;
    LEN_() = 0;
  }
  
  inline ADDR_STRUCT()
  {
    INTEGER_() = 0;
    ADDR_() = -1;
    LEN_() = 0;
  }
  
  inline ADDR_STRUCT(const ADDR_STRUCT& other)
  {
    INTEGER_() = other.INTEGER();
    ADDR_() = other.ADDR();
    LEN_() = other.LEN();
  }

  inline ADDR_STRUCT& operator = (const ADDR_STRUCT& other)
  {
    INTEGER_() = other.INTEGER();
    ADDR_() = other.ADDR();
    LEN_() = other.LEN();
    return *this;
  }

  inline bool operator == (const ADDR_STRUCT& other)const
  {
    return (INTEGER() == other.INTEGER());
  }

  inline bool operator != (const ADDR_STRUCT& other)const
  {
    return (INTEGER() != other.INTEGER());
  }

  inline bool operator < (const ADDR_STRUCT& other)const 
  {
    return (INTEGER() < other.INTEGER());
  }

  inline bool operator > (const ADDR_STRUCT& other)const
  {
    return (INTEGER() > other.INTEGER());
  }

  inline bool operator <= (const ADDR_STRUCT& other)const 
  {
    return (INTEGER() <= other.INTEGER());
  }

  inline bool operator >= (const ADDR_STRUCT& other)const
  {
    return (INTEGER() >= other.INTEGER());
  }

  inline uint32_t operator % (uint32_t e)const
  {
    return (INTEGER() % e);
  }

  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const ADDR_STRUCT& v)
  {
    os<<"<"<<v.INTEGER()<<","<<v.ADDR()<<","<<v.LEN()<<">";

    return os;
  }
  
}
  ;


template<
  uint32_t BUCKET_NUM = 800,
  uint32_t BUF_SIZE = 200000000,//bytes
  class TERM_TYPE = uint32_t
  >
class Sorter
{
  typedef Sorter<BUCKET_NUM, BUF_SIZE, TERM_TYPE> self_t;
  
  typedef FileDataBucket<struct ADDR_STRUCT, 10000> bucket_t;
  
public:
  typedef DynArray<TERM_TYPE> terms_t;

private:
  char *buf_;
  uint32_t p_;
  uint64_t num_;
  std::string filenm_;
  FILE* f_;
  FILE* out_f_;
  
  bucket_t* buckets_[BUCKET_NUM+1];
  
  //std::string filenm_;

  inline bool is_mem_full_(uint32_t s)
  {
    if (p_+s > BUF_SIZE)
      return true;
    return false;
  }

  inline void flush_()
  {
    //std::cout<<p_<<std::endl;
    assert(fwrite(buf_, p_, 1, f_)==1);
    p_=0;
  }

  inline bool is_in_mem_()const
  {
    if (p_+sizeof(uint16_t)>BUF_SIZE)
      return false;

    return (p_+*(uint16_t*)(buf_+p_)+sizeof(uint16_t)<=BUF_SIZE);
  }

  inline void load_()
  {
    //std::cout<<"loading...\n";
    uint32_t i=0;
    for (; p_<BUF_SIZE; ++p_, ++i)
      *(buf_ + i) = *(buf_+p_);
    
    fread(buf_+i, BUF_SIZE-i, 1, out_f_);
    p_ = 0;
  }
  

  uint32_t sort_(const ADDR_STRUCT* array)
  {
    uint32_t r = 0;
    uint32_t min = -1;

    for (uint32_t i=0; i<BUCKET_NUM; ++i)
      if (min>array[i].INTEGER())
      {
        min = array[i].INTEGER();
        r = i;
      }
    
    return r;
  }

  void output_()
  {
    const uint32_t SIZE = 1000000000;
    char* buf = (char*)malloc(SIZE);
          
    FILE* f = fopen((filenm_+".out").c_str(), "w+");
    assert(fwrite(&num_, sizeof(uint64_t), 1, f)==1);

    f_ = fopen(filenm_.c_str(), "r");
    fseek(f_, 0, SEEK_END);
    uint64_t fs = ftell(f_);
    

    char record[4048];
    uint32_t t = fs%SIZE==0?fs/SIZE:fs/SIZE+1;
    for (uint32_t k= 0; k<t; ++k)
    {
      fseek(f_, SIZE*k, SEEK_SET);
      
      fread(buf, SIZE, 1, f_);
      
      uint64_t start = k*SIZE;
      uint64_t end = fs>(k+1)*SIZE? (k+1)*SIZE: fs;
      
      fseek(f, sizeof(uint64_t), SEEK_SET);
      buckets_[BUCKET_NUM]->ready4fetch();
      for (uint64_t i=0; i<buckets_[BUCKET_NUM]->num(); ++i)
      {
        ADDR_STRUCT addr = buckets_[BUCKET_NUM]->next();
        //std::cout<<addr<<std::endl;

        if (addr.ADDR() < start || addr.ADDR() >= end)
        {
          fseek(f, addr.LEN()+sizeof(uint16_t), SEEK_CUR);
          continue;
        }

        //data is in the middle of two buffer
        if (addr.ADDR()+sizeof(uint16_t)+addr.LEN()>end)
        {
          fseek(f_, addr.ADDR()+sizeof(uint16_t), SEEK_SET);

          assert(addr.LEN()+sizeof(uint16_t)<=4048);
          *(uint16_t*)record = addr.LEN();
          assert(fread(record+sizeof(uint16_t), addr.LEN(), 1, f_)==1);

          assert(*(uint32_t*)(record+sizeof(uint16_t)) == addr.INTEGER());
            
          assert(fwrite(record, addr.LEN()+sizeof(uint16_t), 1, f)==1);
          continue;
        }

        assert(*(uint16_t*)(buf+addr.ADDR()-start) == addr.LEN());
        assert(*(uint32_t*)(buf+addr.ADDR()-start+sizeof(uint16_t)) == addr.INTEGER());
        
        assert(fwrite(buf+addr.ADDR()-start, addr.LEN()+sizeof(uint16_t), 1, f)==1);

      }
    }
    
    fclose(f);
    fclose(f_);

    free(buf);

  }
  
public:
  Sorter(const char* nm)
  {
    num_ = 0;
    filenm_ = nm;
    
    f_ = fopen(filenm_.c_str(), "r+");
    if (f_ == NULL)
    {
      f_ = fopen(filenm_.c_str(), "w+");
      assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    }
    else
      assert(fread(&num_, sizeof(uint64_t), 1, f_)==1);
    
    buf_ = (char*)malloc(BUF_SIZE);
    p_ = 0;
    out_f_ = NULL;
  }

  ~Sorter()
  {
    if (buf_!=NULL)
      free(buf_);
    if (out_f_!=NULL)
      fclose(out_f_);
  }

  void ready4add()
  {
    p_ = 0;
    fflush(f_);
    fseek(f_, 0, SEEK_END);

    for (uint32_t i=0; i<BUCKET_NUM+1; ++i)
    {
      std::stringstream ss;
      ss<< (filenm_+".buc.")<<i<<std::endl;
      std::string tmp;
      ss >> tmp;

      buckets_[i] = new bucket_t(tmp.c_str());
      buckets_[i]->ready4add();
    }
  }
  
  void add_terms(const terms_t& terms, uint32_t docid)
  {
    // if (terms.length()<2)
//       return;
    
    for (typename terms_t::size_t i=0; i<terms.length(); ++i)
    {
      uint16_t s = (terms.length()-i)*sizeof(TERM_TYPE)+sizeof(uint32_t);
      if (is_mem_full_(s+sizeof(uint16_t)))
        flush_();

      buckets_[num_%BUCKET_NUM]->push_back(ADDR_STRUCT(terms.at(i), ftell(f_)+p_, s));
      
      *(uint16_t*)(buf_+p_) = s;
      p_ += sizeof(uint16_t);      
      memcpy(buf_+p_, terms.data()+i, (terms.length()-i)*sizeof(TERM_TYPE));
      p_ += (terms.length()-i)*sizeof(TERM_TYPE);
      *(uint32_t*)(buf_+p_) = docid;
      p_ += sizeof(uint32_t);
      
      ++num_;
    }
  }

  void flush()
  {
    std::cout<<"Amount: "<<num_<<std::endl;
    std::cout<<"sorter is flushing...";
    flush_();
    fseek(f_, 0, SEEK_SET);
    assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    fflush(f_);
    fclose(f_);

    for (uint32_t i=0; i<BUCKET_NUM; ++i)
    {
      buckets_[i]->flush();
      buckets_[i]->sort();

      //std::cout<<"Bucket "<<i<<" sorted\n";
    }

    std::cout<<" ...[OK]\n";
  }

  void sort()
  {
    free(buf_);
    buf_= NULL;

//     AlphaSort<uint32_t, false> alpha;
//     alpha.addInputFile(filenm_.c_str());
//     alpha.sort((filenm_+".out").c_str());
//     return;

    for (uint32_t i=0; i<BUCKET_NUM; ++i)
      buckets_[i]->ready4fetch();
    
    ADDR_STRUCT firsts[BUCKET_NUM];
    uint64_t    index[BUCKET_NUM];

    //uint32_t finished = 0;
    for (uint32_t i=0; i<BUCKET_NUM; ++i)
    {
      if (buckets_[i]->num()>0)
        firsts[i] = buckets_[i]->next();
      else
        firsts[i] = -1;
      
      index[i] = 1;
    }

    buckets_[BUCKET_NUM]->ready4add();

    uint32_t last = 0;

    //std::ofstream of("bucket_in.txt");
    while (1)
    {
      last = sort_(firsts);
      assert(last<BUCKET_NUM);
      
      if (firsts[last] == (TERM_TYPE)-1)
        break;
      
      buckets_[BUCKET_NUM]->push_back(firsts[last]);
      //of<<firsts[last].INTEGER()<<" "<<firsts[last].ADDR()<<std::endl;
      
      if (index[last]>=buckets_[last]->num())
      {
        firsts[last] = -1;
        continue;
      }

      firsts[last] = buckets_[last]->next();
      ++index[last];
      // std::cout<<firsts[last]<<"--"<<last;
//       std::cout<<std::endl;
    }

    //std::cout<<buckets_[BUCKET_NUM]->num()<<" ++\n";
    
    for (uint32_t i=0; i<BUCKET_NUM; ++i)
    {
      assert(firsts[i]==(TERM_TYPE)-1);
      buckets_[i]->dump();
      delete buckets_[i];
    }

    buckets_[BUCKET_NUM]->flush();

    std::cout<<"Sorting is done.\nOutputing is going on...\n";

    output_();

    buckets_[BUCKET_NUM]->dump();
    delete buckets_[BUCKET_NUM];

  }

  void ready4fetch()
  {
    if (buf_==NULL)
      buf_ = (char*)malloc(BUF_SIZE);
    
    //out_f_ = fopen((filenm_+".out").c_str(), "r");
    out_f_ = fopen((filenm_+".out").c_str(), "r");
    assert(out_f_!=NULL);
    
    p_ = 0;
    fseek(out_f_, sizeof(uint64_t), SEEK_SET);
    fread(buf_, BUF_SIZE, 1, out_f_);
  }

  inline uint64_t num()const
  {
    return num_;
  }

  void next(terms_t& terms, uint32_t& docid)
  {
    if (!is_in_mem_())
      load_();

    uint32_t s = *(uint16_t*)(buf_+p_);
    p_ += sizeof(uint16_t);
    
    terms.assign(buf_+p_, s - sizeof(uint32_t));
    p_ += terms.size();
    docid = *(uint32_t*)(buf_+p_);
    p_ += sizeof(uint32_t);
  }  

  void output(uint64_t start, uint64_t size)
  {
    std::stringstream ss;
    ss<< (filenm_+".buc.")<<BUCKET_NUM<<std::endl;
    std::string tmp;
    ss >> tmp;

    bucket_t* buckets = new bucket_t(tmp.c_str());

    buckets->ready4fetch();

    for(uint64_t i=0; i<start; ++i)
      buckets->next();

    ss.clear();
    ss<< (filenm_)<<start<<std::endl;
    ss >> tmp;
    FILE* f = fopen(tmp.c_str(), "w+");
    FILE* of = fopen(filenm_.c_str(), "r");
    char buf[1024];
    for (uint32_t i=0; i<size&&i<buckets->num(); ++i)
    {
      ADDR_STRUCT addr = buckets->next();

      uint16_t len;
      fseek(of, addr.ADDR(), SEEK_SET);
      assert(fread(&len, sizeof(uint16_t), 1, of)==1);
      assert(fread(buf, len, 1, of)==1);

      assert(fwrite(&len, sizeof(uint16_t), 1, f)==1);
      assert(fwrite(buf, len, 1, f)==1);
    }

    fclose(f);
    fclose(of);
  }
}
;

NS_IZENELIB_AM_END
#endif

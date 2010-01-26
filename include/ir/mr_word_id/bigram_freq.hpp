/**
   @file ter_freq.hpp
   @author Kevin Hu
   @date 2010.01.08
*/
#ifndef BIGRAM_FREQ_HPP
#define BIGRAM_FREQ_HPP

#include <types.h>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <am/graph_index/dyn_array.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <vector>

NS_IZENELIB_IR_BEGIN
/**
 * @class BigramFrequency
 * @brief BigramFrequency is used for caculating the frequency of bigram.
 *
 **/

template<
  typename IDType = uint32_t
  >
class BigramFrequency
{

  typedef uint32_t FreqType;
  
  struct ID_STRUCT
  {
    char id[sizeof(IDType)];
    char freq[sizeof(FreqType)];

    IDType& ID_()
    {
      return *(IDType*)id;
    }

    FreqType& FREQ_()
    {
      return *(FreqType*)freq;
    }

    IDType ID()const
    {
      return *(IDType*)id;
    }

    FreqType FREQ()const
    {
      return *(FreqType*)freq;
    }
  
    inline ID_STRUCT(IDType i, FreqType j)
    {
      ID_() = i;
      FREQ_() = j;
    }
  
    inline ID_STRUCT(IDType i)
    {
      ID_() = i;
      FREQ_() = 0;
    }
  
    inline ID_STRUCT()
    {
      ID_() = 0;
      FREQ_() = -1;
    }
  
    inline ID_STRUCT(const ID_STRUCT& other)
    {
      ID_() = other.ID();
      FREQ_() = other.FREQ();
    }

    inline ID_STRUCT& operator = (const ID_STRUCT& other)
    {
      ID_() = other.ID();
      FREQ_() = other.FREQ();
      return *this;
    }

    inline bool operator == (const ID_STRUCT& other)const
    {
      return (ID() == other.ID());
    }

    inline bool operator != (const ID_STRUCT& other)const
    {
      return (ID() != other.ID());
    }

    inline bool operator < (const ID_STRUCT& other)const 
    {
      return (FREQ() > other.FREQ());
    }

    inline bool operator > (const ID_STRUCT& other)const
    {
      return (FREQ() < other.FREQ());
    }

    inline bool operator <= (const ID_STRUCT& other)const 
    {
      return (FREQ() >= other.FREQ());
    }

    inline bool operator >= (const ID_STRUCT& other)const
    {
      return (FREQ() <= other.FREQ());
    }

    inline uint32_t operator & (uint32_t e)const
    {
      return (ID() & e);
    }

    /**
     *This is for outputing into std::ostream, say, std::cout.
     **/
  friend std::ostream& operator << (std::ostream& os, const ID_STRUCT& v)
    {
      os<<"<"<<v.ID()<<","<<v.FREQ()<<">";

      return os;
    }
  
  }
    ;

  typedef izenelib::am::DynArray<ID_STRUCT> bucket_t;
  typedef izenelib::am::DynArray<bucket_t*> entry_t;

  void get_threashold_(FILE* f)
  {
    IASSERT(f != NULL);
    fseek(f, 0, SEEK_SET);
    IASSERT(fread(&count_, sizeof(count_), 1, f)==1);
    IASSERT(fread(&threshold_, sizeof(threshold_), 1, f)==1);
    uint32_t fre = 0;
    uint32_t amount = 0;
    
    for (uint32_t i=0; i<count_; ++i)
    {
      uint32_t idx = 0;
      IASSERT(fread(&idx, sizeof(idx), 1, f)==1);

      bucket_t bk;
      bk.load(f);

      amount += bk.length();
      for (uint32_t j=0; j<bk.length(); ++j)
        fre += bk.at(j).FREQ();
    }

    //std::cout<<fre<<"-"<<amount<<std::endl;
    threshold_ =  fre*1./(double)amount;
    std::cout<<"Threshold: "<<threshold_<<std::endl;
  }

protected:
  entry_t entry_;//!< entry of table
  std::string filenm_;
  IDType start_;
  IDType end_;
  uint32_t count_;
  double threshold_;
  

  void clean_()
  {
    for(uint32_t i=0; i<entry_.length(); i++)
    {
      if (entry_.at(i)!= NULL)
        delete (entry_.at(i));
      
      entry_[i] = NULL;
    }

    count_ = 0;
    entry_.clear();
  }
  
public:
  /**
     @brief a constructor
  */
  BigramFrequency(const char* filenm)
    :filenm_(filenm)
  {
    count_ = 0;
    start_ = end_ = 0;
    threshold_ = 0.;
  }
    
  /**
     @brief a destructor
  */
  ~BigramFrequency()
  {
    flush();
  }

  void set_start(uint64_t s)
  {
    if (start_ == (IDType)s)
      return;
    
    start_ = s;

    for(uint32_t i=0; i<entry_.length(); i++)
    {
      if (entry_.at(i)!= NULL)
        delete (entry_.at(i));
      
      entry_[i] = NULL;
    }
    entry_.clear();
  }

  void set_end(uint64_t e)
  {
    end_ = e;
  }
  
  void flush()const
  {
    boost::filesystem::remove(std::string(std::string("rm -f ")+std::string(filenm_+".over")).c_str());
    FILE* f = fopen(std::string(filenm_+".tbl").c_str(), "r+");
    if (f == NULL)
      f = fopen(std::string(filenm_+".tbl").c_str(), "w+");
    
    fseek(f, 0, SEEK_SET);
    IASSERT(fwrite(&count_, sizeof(count_),1, f)==1);
    IASSERT(fwrite(&threshold_, sizeof(threshold_),1, f)==1);
    fseek(f, 0, SEEK_END);
    
    for (uint32_t i=0; i<entry_.length(); ++i)
      if (entry_.at(i)!=NULL)
      {
        uint32_t idx = i+start_;
        IASSERT(fwrite(&idx, sizeof(idx),1, f)==1);
        entry_.at(i)->save(f);
      }

    fclose(f);

    f = fopen(std::string(filenm_+".over").c_str(), "w+");
    fclose(f);
  }

  bool load(uint32_t start = 0, uint32_t end = -1)
  {
    clean_();
    
    FILE* f = fopen(std::string(filenm_+".over").c_str(), "r");
    if (f == NULL)
      return false;
    fclose(f);

    f = fopen(std::string(filenm_+".tbl").c_str(), "r+");
    IASSERT(f != NULL);
    IASSERT(fread(&count_, sizeof(count_), 1, f)==1);
    IASSERT(fread(&threshold_, sizeof(threshold_), 1, f)==1);

    start_ = start;
    end_ = end;
    bucket_t tmp;
    
    for (uint32_t i=0; i<count_; ++i)
    {
      uint32_t idx = 0;
      IASSERT(fread(&idx, sizeof(idx), 1, f)==1);
      if (idx<start_ )
      {
        tmp.load(f);
        continue;
      }
      if (idx>end_)
        return true;

      while (idx >= entry_.length())
        entry_.push_back(NULL);

      entry_[idx] = new bucket_t();
      entry_[idx]->load(f);
    }

    return true;
  }
  
  uint32_t num_items() const
  {
    return count_;
  }

  bool update(uint64_t id1, uint64_t id2, uint32_t freq = 1)
  {
    if (id1 < start_ || id1>end_)
      return false;

    uint32_t idx = id1-start_;
    while (idx >= entry_.length())
      entry_.push_back(NULL);
    
    if (entry_.at(idx) == NULL)
    {
      entry_[idx] = new bucket_t();
      ++count_;
      //std::cout<<count_<<std::endl;
    }

    bucket_t* buk = entry_.at(idx);
    typename bucket_t::size_t i = buk->find(ID_STRUCT(id2));
    
    if (i != bucket_t::NOT_FOUND)
      (*buk)[i].FREQ_()+=freq;
    else
      buk->push_back(ID_STRUCT(id2, freq));
    
    return true;
  }

  uint32_t find(uint64_t id1, uint64_t id2)const
  {
    if (id1<start_ || id1>end_)
      return 0;

    uint32_t idx = id1-start_;
    if (id1>=entry_.length() || entry_.at(idx) == NULL)
      return 0;

    bucket_t* buk = entry_.at(idx);
    typename bucket_t::size_t i = buk->find(ID_STRUCT(id2));
    
    if (i != bucket_t::NOT_FOUND)
      return buk->at(i).FREQ();

    return 0;
  }

  std::vector<IDType> find(uint64_t id1)
  {
    std::vector<IDType> r;
    
    if (id1<start_ || id1>end_)
      return r;

    uint32_t idx = id1-start_;
    if (id1>=entry_.length() || entry_.at(idx) == NULL)
      return r;

    bucket_t* buk = entry_.at(idx);
    if (buk == NULL)
      return r;

    for (uint32_t i=0; i<buk->length(); ++i)
      r.push_back(buk->at(i).ID());

    return r;
  }

  void reset()
  {
    boost::filesystem::remove(std::string(std::string("rm -f ")+std::string(filenm_+".over")).c_str());
  }
  
  double optimize(uint32_t thr = -1)
  {
    clean_();
    
    FILE* f = fopen(std::string(filenm_+".over").c_str(), "r+");
    if (f == NULL)
      return 0;
    fclose(f);

    boost::filesystem::remove(std::string(std::string("rm -f ")+std::string(filenm_+".over")).c_str());
    
    f = fopen(std::string(filenm_+".tbl").c_str(), "r+");
    if (thr == (uint32_t)-1 )
      get_threashold_(f);
    else
    {
      fseek(f, 0, SEEK_SET);
      IASSERT(fread(&count_, sizeof(count_), 1, f)==1);
      threshold_ = thr;
    }
    
    FILE* ff = fopen(std::string(filenm_+".tbl.tmp").c_str(), "w+");
    fseek(ff, sizeof(count_)+sizeof(threshold_), SEEK_SET);
    uint32_t count = 0;
    
    IASSERT(f != NULL);
    fseek(f, sizeof(count_)+sizeof(threshold_), SEEK_SET);
    
    for (uint32_t i=0; i<count_; ++i)
    {
      uint32_t idx = 0;
      IASSERT(fread(&idx, sizeof(idx), 1, f)==1);

      bucket_t bk;
      bk.load(f);
      bk.sort();

      bucket_t obk;
      obk.reserve(bk.length());
      for (uint32_t j=0; j<bk.length(); ++j)
        if (bk.at(j).FREQ()>=threshold_)
          obk.push_back(bk.at(j));
      
      if (obk.length()==0)
        continue;

      IASSERT(fwrite(&idx, sizeof(idx), 1, ff)==1);
      obk.save(ff);
      ++count;
    }

    count_ = count;
    fseek(ff, 0, SEEK_SET);
    IASSERT(fwrite(&count_, sizeof(count_),1, ff)==1);
    IASSERT(fwrite(&threshold_, sizeof(threshold_),1, ff)==1);
    fclose(ff);
    fclose(f);

    boost::filesystem::remove(std::string(filenm_+".tbl").c_str());
    boost::filesystem::rename(std::string(filenm_+".tbl.tmp").c_str(), std::string(filenm_+".tbl").c_str());

    f = fopen(std::string(filenm_+".over").c_str(), "w+");
    fclose(f);

    return threshold_;
  }

  inline double get_threshold()const
  {
    return threshold_;
  }
  
  void display(std::ostream& os=std::cout)
  {
    os<<"count: "<<count_<<std::endl;
    for (uint32_t i=0; i<entry_.length(); ++i)
      if (entry_.at(i)!=NULL)
      {
        os<<"Entry: "<<i<<std::endl;
        os<<(*entry_.at(i))<<std::endl;
      }

    os<<std::endl;
  }
  
};

NS_IZENELIB_IR_END

#endif

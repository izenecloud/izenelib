#ifndef PARTIAL_FP_LIST_HPP
#define PARTIAL_FP_LIST_HPP


#include <string>
#include <cstdio>
#include <ir/dup_det/integer_dyn_array.hpp>

NS_IZENELIB_IR_BEGIN

template<
  typename UNIT_TYPE = uint64_t,
  uint8_t  FP_LENGTH = 6,
  uint64_t CACHE_SIZE = 600
  >
class PartialFpList
{
  
public:
  typedef izenelib::am::IntegerDynArray<UNIT_TYPE> FpVector;
  
  typedef izenelib::am::IntegerDynArray<uint32_t> DocIDVector;

  typedef DocIDVector::size_t size_t;
  

private:
  
  enum CACHE_STATUS 
    {
      ADD_DOCS, UNIFORM_ACCESS
    }
    ;

  
  
  std::string file_name_;
  size_t doc_num_;        //<! Total amount of documents.
  size_t in_mem_doc_num_; //!< Amount of documents in memory.
  size_t start_doc_i_;    //!< start index of document in memory.
  CACHE_STATUS cache_status_; //!< Cache status is adding docs and reading docs.  
  
  const size_t ALL_INFO_IN_MEM_LENGTH;
  const uint8_t  UNIT_LEN;       //!< how many units do pre-clustering use.
  const uint8_t PARTIALS_SIZE;

  FILE** fhandlers_;
  FpVector**    fp_ptrs_;
  DocIDVector   docids_;

protected:
  inline void clean_fp_ptr()
  {
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
      fp_ptrs_[i] = NULL;
  }

  inline void swith_cache_status(CACHE_STATUS sta)
  {    
    if (cache_status_ == UNIFORM_ACCESS)
    {
      cache_status_ = ADD_DOCS;
      for (uint8_t i =0; i<PARTIALS_SIZE; i++)
      {
        if (fp_ptrs_[i]!=NULL)
          fp_ptrs_[i]->reset();
      }
      in_mem_doc_num_ = 0;
      start_doc_i_ = doc_num_;
    }
    else
    {
      cache_status_ = UNIFORM_ACCESS;
      unload_fps();
      // for (uint8_t i =0; i<FP_LENGTH; i++)
//       {
//         start_fps_[i] = start_doc_i_;
//       }
      
    }
  }
  
  inline void set_cache_status(CACHE_STATUS sta)
  {
    if (sta == cache_status_)
      return;
    swith_cache_status(sta);
  }

  /**
     This function is to indicate if the cache is full when adding docs.
   **/
  inline bool cache_full()
  {
    assert (cache_status_ == ADD_DOCS);

    return in_mem_doc_num_ >= ALL_INFO_IN_MEM_LENGTH;
  }
  
  inline void unload_fps()
  {
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
    {
      if (fp_ptrs_[i]==NULL || fp_ptrs_[i]->length()==0)
        continue;

      //std::cout<<"unload seeking: "<<start_doc_i_<<std::endl;
      
      fseek(fhandlers_[i], start_doc_i_*sizeof(UNIT_TYPE)*UNIT_LEN, SEEK_SET);
      assert(fwrite(fp_ptrs_[i]->data(), fp_ptrs_[i]->size(), 1, fhandlers_[i])==1);
      fp_ptrs_[i]->compact();
      fp_ptrs_[i]->reset();
    }
  }

  inline void clean_fp()
  {
    //std::cout<<"clean_fp....\n";
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
    {
      if (fp_ptrs_[i]==NULL)
        continue;
      delete (fp_ptrs_[i]);
      fp_ptrs_[i] = NULL;
    }
  }
  
  inline void load_fp(uint8_t fpi, size_t n)
  {
    assert(fpi < PARTIALS_SIZE);
    
    static size_t max_fps_size = 0;
    max_fps_size = (CACHE_SIZE*1000000 - docids_.size())/sizeof(UNIT_TYPE);
    max_fps_size += max_fps_size % UNIT_LEN;

    start_doc_i_ = n;
    size_t size = (doc_num_ - n)*UNIT_LEN;
    size = size > max_fps_size? max_fps_size: size;
    
    std::cout<<size<<" "<<max_fps_size<<" "<<doc_num_<<"  loaded fp size\n";

//     for (uint8_t i =0; i<PARTIALS_SIZE; i++)
//     {
//       if (fpi == i || fp_ptrs_[i]==NULL)
//         continue;
//       delete fp_ptrs_[i];
//       fp_ptrs_[i] = NULL;
//     }
    
    if (fp_ptrs_[fpi] == NULL)
      fp_ptrs_[fpi] = new FpVector();

    fseek(fhandlers_[fpi], n*sizeof (UNIT_TYPE)*UNIT_LEN, SEEK_SET);
    assert(fread(fp_ptrs_[fpi]->array(size), size*sizeof(UNIT_TYPE), 1, fhandlers_[fpi])==1);
  }

  inline bool is_in_mem(uint8_t fpi, size_t n)
  {
    if (fp_ptrs_[fpi]!=NULL && n>=start_doc_i_ && n < start_doc_i_+fp_ptrs_[fpi]->length()/UNIT_LEN)
      return true;
    
    return false;
  }
  
  
public:
  explicit PartialFpList(const char* file_name, uint8_t unit_len=2, size_t doc_num = 0)
    :file_name_(file_name),doc_num_(doc_num), in_mem_doc_num_(0),start_doc_i_(0),cache_status_(ADD_DOCS),
     ALL_INFO_IN_MEM_LENGTH(CACHE_SIZE*1000000/(FP_LENGTH*sizeof(UNIT_TYPE))),
     UNIT_LEN(unit_len)
    , PARTIALS_SIZE((uint8_t)((double)FP_LENGTH/UNIT_LEN+0.5)) 
  {
    fp_ptrs_ = new FpVector*[PARTIALS_SIZE];
    fhandlers_ = new FILE*[PARTIALS_SIZE+1];
    clean_fp_ptr();
    
    static char num[8];
    
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
    {
      sprintf(num, ".fp.%d", i);
      std::string s = file_name_;
      s += num;
      fhandlers_[i] = fopen(s.c_str(), "r+");
      if (fhandlers_[i] == NULL)
        fhandlers_[i] = fopen(s.c_str(), "w+");
      if (fhandlers_[i] == NULL)
      {
        std::cout<<"Can't create file: "<<s<<std::endl;
        return;
      }
    }

    std::string s = file_name_;
    s += ".id";
    fhandlers_[PARTIALS_SIZE] = fopen(s.c_str(), "r+");
    if (fhandlers_[PARTIALS_SIZE] != NULL)
    {
      fread(&doc_num_, sizeof(size_t), 1, fhandlers_[PARTIALS_SIZE]);
      if (doc_num_!= 0)
        fread(docids_.array(doc_num_), doc_num_*sizeof(uint32_t), 1, fhandlers_[PARTIALS_SIZE]);
    }
    else{
      fhandlers_[PARTIALS_SIZE] = fopen(s.c_str(), "w+");
      if (fhandlers_[PARTIALS_SIZE] == NULL)
      {
        std::cout<<"Can't create file: "<<s<<std::endl;
        return;
      }
      fwrite(&doc_num_, sizeof(size_t), 1, fhandlers_[PARTIALS_SIZE]);
    }

    start_doc_i_ = doc_num_;
    std::cout<<"ALL_INFO_IN_MEM_LENGTH: "<<ALL_INFO_IN_MEM_LENGTH<<std::endl;
  }

  inline ~PartialFpList()
  {
    flush();
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)      
      fclose(fhandlers_[i]);
    clean_fp();

    delete fhandlers_;
    delete fp_ptrs_;
  }
  
  size_t add_doc(uint32_t docid, const FpVector& fp)
  {
    //std::cout<<"*** "<<fp<<std::endl;
    
    set_cache_status(ADD_DOCS);
    
    if (cache_full())
    {
      unload_fps();
      in_mem_doc_num_ = 0;
      start_doc_i_ = doc_num_;
    }
    
    assert(FP_LENGTH == fp.length());
    docids_.push_back(docid);
    
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
    {
      if (fp_ptrs_[i]==NULL)
        fp_ptrs_[i] = new FpVector(10);
      
      for (uint8_t j=0; j<UNIT_LEN; j++)
      {
        size_t ii = i*UNIT_LEN+j;
        fp_ptrs_[i]->push_back(ii<fp.length()?fp[ii]: 0);
      }
    }

    ++in_mem_doc_num_;
    return ++doc_num_;
  }

  inline void ready_for_uniform_access(uint8_t fpi)
  {
    set_cache_status(UNIFORM_ACCESS);
    clean_fp();
    load_fp(fpi, 0);
  }

  inline UNIT_TYPE get_fp(uint8_t fpi, size_t n, uint8_t i=0)
  {
    assert(fpi<PARTIALS_SIZE);
    if (!is_in_mem(fpi, n))
      load_fp(fpi, n);
    
    return (*fp_ptrs_[fpi]).at((n-start_doc_i_)*UNIT_LEN+i);
  }

  inline void flush()
  {
    if (cache_status_ == ADD_DOCS)
    {
      unload_fps();
    }
    
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
      fflush(fhandlers_[i]);

    fseek(fhandlers_[PARTIALS_SIZE], 0, SEEK_SET);
    fwrite(&doc_num_, sizeof (size_t), 1, fhandlers_[PARTIALS_SIZE]);
    fwrite(docids_.data(), docids_.size(), 1, fhandlers_[PARTIALS_SIZE]);
    fflush(fhandlers_[PARTIALS_SIZE]);
    
    //std::cout<<docids_.length()<<" "<<docids_.size()<<std::endl;
  }

  inline size_t doc_num() const
  {
    return doc_num_;
  }
  
  void reset()
  {
    static char num[8];
    for (uint8_t i =0; i<PARTIALS_SIZE; i++)
    {
      sprintf(num, ".fp.%d", i);
      std::string s = file_name_;
      s += num;
      fclose(fhandlers_[i]);
      fhandlers_[i] = fopen(s.c_str(), "w+");
      if (fhandlers_[i] == NULL)
      {
        std::cout<<"Can't create file: "<<s<<std::endl;
        return;
      }
    }

    std::string s = file_name_;
    s += ".id";
    fclose(fhandlers_[PARTIALS_SIZE]);
    fhandlers_[PARTIALS_SIZE] = fopen(s.c_str(), "w+");
    if (fhandlers_[PARTIALS_SIZE] == NULL)
    {
      std::cout<<"Can't create file: "<<s<<std::endl;
      return;
    }

    doc_num_ = 0;
    fwrite(&doc_num_, sizeof(size_t), 1, fhandlers_[PARTIALS_SIZE]);
    
    in_mem_doc_num_ = 0;
    start_doc_i_ = 0;
    clean_fp();
  }

  inline uint8_t unit_len()const
  {
    return UNIT_LEN;
  }

  inline void free()
  {
    clean_fp();
    docids_.compact();
    start_doc_i_ = doc_num_;
    in_mem_doc_num_ = 0;
  }

  inline uint32_t& operator[] (size_t i)
  {
    assert(i < docids_.length());
    return docids_[i];
  }
  
  void print()const
  {
    //std::cout<<docids_<<std::endl;
    
    if (cache_status_ == ADD_DOCS)
    {
      std::cout<<"Adding docs...\n";
      for (size_t i=0; i<in_mem_doc_num_; i++)
      {
        std::cout<<(*this)[i]<<"# ";//std::cout<<docids_.at(i+start_doc_i_)<<"# ";
        for (uint8_t j=0; j<PARTIALS_SIZE; j++)
          if (fp_ptrs_[j]!= NULL)
          { //std::cout<<(*fp_ptrs_[j]).length()<<" "<<(int)j<<std::endl;
            for (uint8_t k=0; k<UNIT_LEN; k++)
              std::cout<<(*fp_ptrs_[j])[i*UNIT_LEN+k]<<" ";
            std::cout<<"| ";
          }
        
            
        std::cout<<std::endl;
      }
    }
    else
    {
      std::cout<<"Uniform access to docs...\n";
      for (uint8_t i = 0; i<PARTIALS_SIZE; i++)
      {
        if (fp_ptrs_[i] == NULL)
          continue;
        std::cout<<"------------- #"<<(int)i<<"  fingerprinting("<<fp_ptrs_[i]->length()/UNIT_LEN<<") ------------------\n";
        for (size_t j=0; j<fp_ptrs_[i]->length(); j++)
        {
          std::cout<<j/UNIT_LEN+start_doc_i_<<": "<<(*fp_ptrs_[i])[j]<<" ";
          if ((j+1)%UNIT_LEN==0)
            std::cout<<"\n";
        } 
      } 
    }  
  }
  
}
  ;

NS_IZENELIB_IR_END
#endif


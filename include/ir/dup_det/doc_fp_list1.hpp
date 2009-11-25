/**
   @file doc_fp_list1.hpp
   @author Kevin
   @date 2009.11.25
   @brief it's not in use anymore.
 */
#ifndef DOC_FP_LIST_HPP
#define DOC_FP_LIST_HPP

#include <types.h>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <string>

NS_IZENELIB_IR_BEGIN

template <
  uint64_t CACHE_SIZE = 600, //MB
  uint8_t  FP_LENGTH = 6,
  uint8_t  THRESHOLD = 2
  >
class DocFpList
{

#define REPEAT_1 u = (u-1)&u;if (u == 0)return true;
#define REPEAT_2 REPEAT_1 REPEAT_1
#define REPEAT_3 REPEAT_2 REPEAT_1
#define REPEAT_4 REPEAT_2 REPEAT_2
  #define REPEAT_5 REPEAT_2 REPEAT_3
  #define REPEAT_6 REPEAT_3 REPEAT_3
#define WHILE(n) REPEAT_##n
  
public:
  typedef izenelib::am::IntegerDynArray<uint32_t> DocIDVector;
  typedef izenelib::am::IntegerDynArray<uint64_t> FpVector;


private:
  enum CACHE_STATUS 
    {
      ADD_DOCS, UNIFORM_ACCESS
    }
    ;

  DocIDVector docids_;
  FpVector*   fp_ptrs_[FP_LENGTH];
  std::string file_name_;
  uint64_t doc_num_;        //<! Total amount of documents.
  uint64_t in_mem_doc_num_; //!< Amount of documents in memory.
  uint64_t start_doc_i_;    //!< start index of document in memory.
  const uint64_t ALL_INFO_IN_MEM_LENGTH;

  ///---------------------------------------
  //uint64_t start_fps_[FP_LENGTH];
  //const uint64_t REMAIN_CACHE_SIZE;//!< the size of cache excluding docids'
  
  CACHE_STATUS cache_status_; //!< Cache status is adding docs and reading docs.
  FILE* fhandlers_[FP_LENGTH+1];

protected:
  inline void clean_fp_ptr()
  {
    for (uint8_t i =0; i<FP_LENGTH; i++)
      fp_ptrs_[i] = NULL;

    // for (uint8_t i =0; i<FP_LENGTH; i++)
//       start_fps_[i] = 0;
  }

  inline void swith_cache_status(CACHE_STATUS sta)
  {    
    if (cache_status_ == UNIFORM_ACCESS)
    {
      cache_status_ = ADD_DOCS;
      for (uint8_t i =0; i<FP_LENGTH; i++)
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
      docids_.compact();
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
    IASSERT (cache_status_ == ADD_DOCS);

    return in_mem_doc_num_ >= ALL_INFO_IN_MEM_LENGTH;
  }
  
  inline void unload_fps()
  {
    for (uint8_t i =0; i<FP_LENGTH; i++)
    {
      if (fp_ptrs_[i]==NULL)
        continue;
      
      fseek(fhandlers_[i], start_doc_i_*sizeof(uint64_t), SEEK_SET);
      fwrite(fp_ptrs_[i]->data(), fp_ptrs_[i]->size(), 1, fhandlers_[i]);
      fp_ptrs_[i]->compact();
      fp_ptrs_[i]->reset();
    }
  }

  inline void clean_fp()
  {
    for (uint8_t i =0; i<FP_LENGTH; i++)
    {
      if (fp_ptrs_[i]==NULL)
        continue;
      delete fp_ptrs_[i];
      fp_ptrs_[i] = NULL;
    }
  }
  
  inline void load_fp(uint8_t fpi, uint64_t n)
  {
    static uint64_t max_fps_size = 0;
    max_fps_size = (CACHE_SIZE*1000000-docids_.size())/sizeof(uint64_t);
    
    fseek(fhandlers_[fpi], n*sizeof (uint64_t), SEEK_SET);
    start_doc_i_ = n;
    //start_fps_[fpi] = n;
    uint64_t size = doc_num_ - n;
    size = size > max_fps_size? max_fps_size: size;
    
    //std::cout<<size<<"  loaded fp size\n";
    
    if (fp_ptrs_[fpi]==NULL)
      fp_ptrs_[fpi] = new FpVector(size);
    
    fread(fp_ptrs_[fpi]->array(size), size*sizeof(uint64_t), 1, fhandlers_[fpi]);    
  }

  inline bool is_in_mem(uint8_t fpi, uint64_t n)
  {
    if (fp_ptrs_[fpi]!=NULL && n>=start_doc_i_ && n<start_doc_i_+fp_ptrs_[fpi]->length())
      return true;
    
    return false;
  }
  
  
public:
  explicit DocFpList(const char* file_name)
    :file_name_(file_name),doc_num_(0), in_mem_doc_num_(0),start_doc_i_(0),
     ALL_INFO_IN_MEM_LENGTH(CACHE_SIZE*1000000/(sizeof(uint32_t)+FP_LENGTH*sizeof(uint64_t)))
    , cache_status_(ADD_DOCS)
  {
    clean_fp_ptr();
    static char num[5];
    
    for (uint8_t i =0; i<FP_LENGTH; i++)
    {
      sprintf(num, ".%d", i);
      std::string s = file_name_;
      s += num;
      fhandlers_[i] = fopen(s.c_str(), "r+");
      if (fhandlers_[i] == NULL)
        fhandlers_[i] = fopen(s.c_str(), "w");
      if (fhandlers_[i] == NULL)
      {
        std::cout<<"Can't create file: "<<s<<std::endl;
        return;
      }
    }

    std::string s = file_name_;
    s += ".id";
    fhandlers_[FP_LENGTH] = fopen(s.c_str(), "r+");
    if (fhandlers_[FP_LENGTH] != NULL)
    {
      fread(&doc_num_, sizeof(uint64_t), 1, fhandlers_[FP_LENGTH]);
      if (doc_num_!= 0)
        fread(docids_.array(doc_num_), doc_num_*sizeof(uint32_t), 1, fhandlers_[FP_LENGTH]);
    }
    else{
      fhandlers_[FP_LENGTH] = fopen(s.c_str(), "w");
      if (fhandlers_[FP_LENGTH] == NULL)
      {
        std::cout<<"Can't create file: "<<s<<std::endl;
        return;
      }
      fwrite(&doc_num_, sizeof(uint64_t), 1, fhandlers_[FP_LENGTH]);
    }

    start_doc_i_ = doc_num_;
    std::cout<<"ALL_INFO_IN_MEM_LENGTH: "<<ALL_INFO_IN_MEM_LENGTH<<std::endl;
  }

  inline ~DocFpList()
  {
    flush();
    for (uint8_t i =0; i<FP_LENGTH; i++)
    {
      if (fp_ptrs_[i]!= NULL)
        delete fp_ptrs_[i];
      fclose(fhandlers_[i]);
    }

    fclose(fhandlers_[FP_LENGTH]);
    
  }

  inline uint64_t doc_num() const
  {
    return doc_num_;
  }
  
  uint64_t add_doc(uint32_t docid, const FpVector& fp)
  {
    set_cache_status(ADD_DOCS);
    
//     if (cache_full())
//     {
//       unload_fps();
//       in_mem_doc_num_ = 0;
//       start_doc_i_ = doc_num_;
//     }

    docids_.push_back(docid);

//     IASSERT(FP_LENGTH == fp.length());
    
//     for (uint8_t i =0; i<FP_LENGTH; i++)
//     {
//       if (fp_ptrs_[i]==NULL)
//         fp_ptrs_[i] = new FpVector(10);
//       fp_ptrs_[i]->push_back(fp[i]);
//     }

//     ++in_mem_doc_num_;
    return ++doc_num_;
  }

  inline void ready_for_uniform_access(uint8_t fpi)
  {
    set_cache_status(UNIFORM_ACCESS);
    clean_fp();
    load_fp(fpi, 0);
    
    IASSERT((*fp_ptrs_[fpi]).length()==doc_num_);
    //return *fp_ptrs_[fpi];
  }

  inline uint64_t get_fp(uint8_t fpi, uint64_t i)const
  {
    return (*fp_ptrs_[fpi]).at(i);
  }
  
  
  inline bool is_similar(uint64_t i, uint64_t j)const
  {
    uint64_t u = i^j;
    if (u == 0)
      return true;

    WHILE(2);
    
//     uint8_t k = 0;
//     while (k<THRESHOLD)
//     {
//       ++k;
      
//       if (u == 0)
//         return true;
//     }
    
    return false;
  }
  
  inline uint32_t get_docid(size_t i)
  {
    return docids_[i];
  }

  inline void flush()
  {
    if (cache_status_ == ADD_DOCS)
    {
      unload_fps();
    }
    
    for (uint8_t i =0; i<FP_LENGTH; i++)
      fflush(fhandlers_[i]);

    fseek(fhandlers_[FP_LENGTH], 0, SEEK_SET);
    fwrite(&doc_num_, sizeof (uint64_t), 1, fhandlers_[FP_LENGTH]);
    fwrite(docids_.data(), docids_.size(), 1, fhandlers_[FP_LENGTH]);
    std::cout<<docids_.length()<<" "<<docids_.size()<<std::endl;
    fflush(fhandlers_[FP_LENGTH]);
  }

  void reset()
  {
    static char num[5];
    for (uint8_t i =0; i<FP_LENGTH; i++)
    {
      sprintf(num, ".%d", i);
      std::string s = file_name_;
      s += num;
      fclose(fhandlers_[i]);
      fhandlers_[i] = fopen(s.c_str(), "w");
      if (fhandlers_[i] == NULL)
      {
        std::cout<<"Can't create file: "<<s<<std::endl;
        return;
      }
    }
    std::string s = file_name_;
    s += ".id";
    fclose(fhandlers_[FP_LENGTH]);
    fhandlers_[FP_LENGTH] = fopen(s.c_str(), "w");
    if (fhandlers_[FP_LENGTH] == NULL)
    {
      std::cout<<"Can't create file: "<<s<<std::endl;
      return;
    }

    doc_num_ = 0;
    fwrite(&doc_num_, sizeof(uint64_t), 1, fhandlers_[FP_LENGTH]);
    start_doc_i_ = 0;
    in_mem_doc_num_ = 0;
    docids_.reset();
    clean_fp();
  }
  
  void print()
  {
    if (cache_status_ == ADD_DOCS)
    {
      std::cout<<"Adding docs...\n";
      for (size_t i=0; i<in_mem_doc_num_; i++)
    {
      std::cout<<"ID:"<<docids_[start_doc_i_+i]<<" FP:  ";
      for (uint8_t j=0; j<FP_LENGTH; j++)
        if (fp_ptrs_[j]!= NULL)
          std::cout<<(*fp_ptrs_[j])[i]<<" ";
      std::cout<<std::endl;
    }
    }
    else
    {
      std::cout<<"Uniform access to docs...\n";
      for (uint8_t i = 0; i<FP_LENGTH; i++)
      {
        if (fp_ptrs_[i] == NULL)
          continue;
        std::cout<<"------------- #"<<(int)i<<"  fingerprinting("<<fp_ptrs_[i]->length()<<") ------------------\n";
        for (size_t j=0; j<fp_ptrs_[i]->length(); j++)
          std::cout<<j+start_doc_i_<<": "<<(*fp_ptrs_[i])[j]<<std::endl;
      }
      
    }
    
    
  }
  
}
  ;

NS_IZENELIB_IR_END
#endif


#ifndef INDEX_FP_HPP
#define INDEX_FP_HPP


#include <types.h>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <string>
#include <ir/dup_det/partial_fp_list.hpp>
#include <ir/dup_det/hash_table.hpp>
#include <ir/dup_det/hash_group.hpp>
#include <ir/dup_det/fp_hash_table.hpp>
#include <ir/dup_det/group_table.hpp>

NS_IZENELIB_IR_BEGIN

template <
  uint8_t  FP_HASH_NUM = 1,
  class    UNIT_TYPE = uint64_t,
  uint8_t  FP_LENGTH = 6,
  uint32_t CACHE_SIZE = 600,//MB
  uint32_t ENTRY_SIZE = 1000000
  >
class FpIndex
{
  typedef PartialFpList<UNIT_TYPE, FP_LENGTH, CACHE_SIZE> FpList;
  typedef HashTable<ENTRY_SIZE, UNIT_TYPE> HashT;
  typedef GroupTable<ENTRY_SIZE, UNIT_TYPE> GroupT;
  typedef FpHashTable<CACHE_SIZE/FP_HASH_NUM, FP_LENGTH*sizeof(UNIT_TYPE), ENTRY_SIZE > FpHashT;
  
  typedef izenelib::am::IntegerDynArray<uint8_t> Vector8;
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;
  typedef izenelib::am::IntegerDynArray<Vector32*> Vector32Ptr;
  typedef Vector32::size_t size_t;
  
protected:
  const uint8_t UNIT_LEN_;
  FpList* fp_list_;
  HashT**  ht_ptrs_;
  GroupT* group_;
  FpHashT* fp_hash_ptrs_[FP_HASH_NUM];
  std::string filenm_;
  

  inline bool border_compare(const uint64_t* p1, const uint64_t* p2, uint8_t threshold = 2)
  {
    //std::cout<<"\nborder_compare: ";
    uint8_t r = 0;
    for (uint8_t i=0; i<FP_LENGTH; i++)
    {
      //std::cout<<p1[i]<<"-"<<p2[i]<<" ";
      
      if (p1[i]!=p2[i])
        ++r;
      
      if (r>threshold)
        return false;
    }
    //std::cout<<std::endl;
    
    return true;
  }
  
  
public:
  inline FpIndex(const char* filenm, uint8_t unit_len=2)
    : UNIT_LEN_(unit_len)
  {
    filenm_ = filenm;
    
    std::string s = filenm_;
    s+= ".group";
    group_ = new GroupT(s.c_str());

    
    fp_list_ = NULL;

    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      fp_hash_ptrs_[i] = NULL;
  }

  inline ~FpIndex()
  {
    if (fp_list_!= NULL)
    {
      fp_list_->flush();
      delete fp_list_;
    }

    if (group_!=NULL)
    {
      group_->flush();
      delete group_;
    }

    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      if (fp_hash_ptrs_[i]!= NULL)
    {
      fp_hash_ptrs_[i]->flush();
      delete fp_hash_ptrs_[i];
    }
  }

  inline void flush()
  {
    if (fp_list_!= NULL)
      fp_list_->flush();
    if (group_!=NULL)
      group_->flush();
    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      if (fp_hash_ptrs_[i]!= NULL)
        fp_hash_ptrs_[i]->flush();
  }

  inline void ready_for_insert()
  {
    std::string s = filenm_;
    s+= ".fphash";

    for (uint8_t i=0; i<FP_HASH_NUM; i++)
    {
      std::string ss = s;
      ss += '.';
      ss += i+'0';
      if (fp_hash_ptrs_[i]==NULL)
        fp_hash_ptrs_[i] = new FpHashT(ss.c_str());
      fp_hash_ptrs_[i]->ready_for_insert();
    }

    s = filenm_;
    s += ".fplist";
    fp_list_ = new FpList(s.c_str(), UNIT_LEN_);
  }

  inline void ready_for_find()
  {
  }
  
  inline void add_doc(size_t docid, const Vector64& fp)
  {
    fp_list_->add_doc(docid, fp);
    
    docid = fp_list_->doc_num()-1;
    
    fp_hash_ptrs_[docid%FP_HASH_NUM]->add_doc(docid/FP_HASH_NUM, fp);
  }

  inline void indexing()
  {
    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      if (fp_hash_ptrs_[i]!= NULL)
    {
      fp_hash_ptrs_[i]->flush();
      delete fp_hash_ptrs_[i];
      fp_hash_ptrs_[i] = NULL;
    }

    ht_ptrs_ = new HashT*[FP_LENGTH/UNIT_LEN_];
    for (uint8_t i=0; i<FP_LENGTH/UNIT_LEN_; i++)
    {
      std::string s = filenm_;
      s += ".hasht";
      s += i+'0';
      ht_ptrs_[i] = new HashT(s.c_str());
    }

    for (uint8_t i=0; i<FP_LENGTH/UNIT_LEN_; i++)
    {
      fp_list_->ready_for_uniform_access(i);
      for (size_t j=0; j<fp_list_->doc_num();j++)
      {
        uint64_t v = 0;
        for (uint8_t k=0; k<UNIT_LEN_; k++)
          v  = 37*v + fp_list_->get_fp(i, j, k);
    
        ht_ptrs_[i]->push_back(v);
      }
      ht_ptrs_[i]->compact();
    }

    fp_list_->free();
    std::cout<<"\nPre-clustering is over!(1.4GB)\n";
    getchar();

    //----------------------------------------
    
    std::string s = filenm_;
    s+= ".fphash";
    for (uint8_t i=0; i<FP_HASH_NUM; i++)
    {
      std::string ss = s;
      ss += '.';
      ss += i+'0';
      if (fp_hash_ptrs_[i]==NULL)
        fp_hash_ptrs_[i] = new FpHashT(ss.c_str());
      fp_hash_ptrs_[i]->ready_for_find();
    }

    Vector8 flags;
    for (uint32_t i=0; i<fp_list_->doc_num();i++)
    {
      uint32_t docid1 = i;//fps[i];
      memset(flags.array(fp_list_->doc_num()), fp_list_->doc_num(), 0);
      const uint64_t* p1 = fp_hash_ptrs_[docid1%FP_HASH_NUM]->find(docid1/FP_HASH_NUM);

      for (uint8_t k=0; k<FP_LENGTH/UNIT_LEN_; k++)
      {
        const Vector32& v = (*(ht_ptrs_[k]))[i];
      
        for (size_t j = 0; j<v.length(); j++)
        {
          size_t index = v.at(j);
          if (v.at(j) <= i)
            continue;

          uint32_t docid2 = index;//fps[index];
          if (flags.at(docid2)==1 || group_->same_group(docid1, docid2))
            continue;

          const uint64_t* p2 = fp_hash_ptrs_[docid2%FP_HASH_NUM]->find(docid2/FP_HASH_NUM);
          if (p1!=NULL && p2!=NULL && border_compare(p1,p2))
          {
            group_->add_doc(docid1, docid2);
            //std::cout<<docid1<<" "<<docid2<<std::endl;
          }
          
          flags[docid2] = 1;
        }
      }

      if (i%1000000==0)
        std::cout<<(double)i/fp_list_->doc_num()*100.<<"% ...\n";
    }

    group_->compact();
    //std::cout<<*group_<<std::endl;
  }
  
  inline const uint64_t* find(size_t docid)
  {
    return NULL;
  }
  
  inline size_t doc_num()const
  {
    return fp_list_->doc_num();
  }
  
}
  ;

NS_IZENELIB_IR_END

#endif

/**
   @file index_fp.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef INDEX_FP_HPP
#define INDEX_FP_HPP

#include <ir/dup_det/partial_fp_list.hpp>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <types.h>
#include <string>
#include <ir/dup_det/hash_table.hpp>
#include <ir/dup_det/hash_group.hpp>
#include <ir/dup_det/fp_hash_table.hpp>
#include <ir/dup_det/group_table.hpp>
#include <ir/dup_det/prime_gen.hpp>
#include <sys/time.h>
#include <math.h>

//#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>


NS_IZENELIB_IR_BEGIN

/**
   @class FpIndex
   @brief It's an indexer that provides fast access of grouped fingerprinting.
 */
template <
  uint8_t  FP_HASH_NUM = 1,//!< number of hash tables
  typename    UNIT_TYPE = uint64_t,//!< the minimous data type for pre-clustering
  uint8_t  FP_LENGTH = 6,//!< number of minimous data unit
  uint32_t CACHE_SIZE = 200,//!< cache size
  uint32_t ENTRY_SIZE = 1000000
  >
class FpIndex
{
  typedef PartialFpList<UNIT_TYPE, FP_LENGTH, CACHE_SIZE> FpList;
  typedef HashTable<ENTRY_SIZE, UNIT_TYPE> HashT;
  typedef GroupTable<ENTRY_SIZE, UNIT_TYPE> GroupT;
  typedef FpHashTable<UNIT_TYPE, CACHE_SIZE/FP_HASH_NUM, FP_LENGTH*sizeof(UNIT_TYPE), ENTRY_SIZE > FpHashT;
  
  typedef izenelib::am::IntegerDynArray<uint8_t> Vector8;
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;
  typedef izenelib::am::IntegerDynArray<Vector32*> Vector32Ptr;
  typedef Vector32::size_t size_t;
  typedef izenelib::ir::PrimeGen<> Prime;
  
protected:
  const uint8_t UNIT_LEN_;//!< number of minimous data type that's taken as a unit
  FpList* fp_list_;//!< partial FP list
  HashT**  ht_ptrs_;//!< pre-clustering hash table
  GroupT* group_;//!< documents group with inner id
  GroupT* final_group_;//document group with real doc id
  FpHashT* fp_hash_ptrs_[FP_HASH_NUM];//!< docid to address of FP hash table
  Vector32Ptr docid_hash_;//!< docid hash table
  std::string filenm_;//!< prefix of file name.
  mutable boost::mutex mtx_;//!< it's for indexing in case of query
  //  static Prime* prime_;
  

  inline bool broder_compare(const UNIT_TYPE* p1, const UNIT_TYPE* p2, uint8_t threshold = 1)
  {
    //std::cout<<"\nborder_compare: ";
    uint8_t r = 0;
    for (uint8_t i=0; i<FP_LENGTH/(sizeof(UNIT_TYPE)/sizeof(UNIT_TYPE)); i++)
    {
      //std::cout<<p1[i]<<"-"<<p2[i]<<" ";
      
      if (p1[i]==p2[i])
        ++r;
      
      if (r>=threshold)
        return true;
    }
    //std::cout<<std::endl;
    
    return false;
  }

  inline bool charick_compare(const UNIT_TYPE* p1, const UNIT_TYPE* p2, uint8_t threshold = 50)
  {
    uint32_t count = 0;
    for (uint32_t i=0; i<FP_LENGTH; ++i)
    {
      UNIT_TYPE k = p1[i]^p2[i];
      while(k)
      {
        k &= (k-1);
        ++count;
      }

      if (count>=threshold)
        return false;
    }
    
    return true;
  }

  inline void switch_docid_in_group()
  {
    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      if (fp_hash_ptrs_[i]!= NULL)
    {
      fp_hash_ptrs_[i]->flush();
      delete fp_hash_ptrs_[i];
      fp_hash_ptrs_[i] = NULL;
    }

    std::string s = filenm_;
    if(!fp_list_)
    {
      s = filenm_;
      s += ".fplist";
      fp_list_ = new FpList(s.c_str(), UNIT_LEN_);
    }

    boost::mutex::scoped_lock lock(mtx_);
    
    IASSERT(final_group_!=NULL);

    {
      s = filenm_;
      s += ".final_group";
      final_group_->reset(s.c_str());
    }

    IASSERT(group_!=NULL);
    final_group_->assign(*group_);
    delete group_;
    group_ = NULL;

    final_group_->set_docid(*fp_list_);
    final_group_->flush();
  }

  /**
     @brief reset docid hash table, clear the table
   */
  inline void reset_docid_hash()
  {
    for (uint32_t i=0 ;i<docid_hash_.length(); ++i)
      if (docid_hash_.at(i) != NULL)
      {
        delete docid_hash_.at(i);
        docid_hash_[i] = NULL;
      }

    if (docid_hash_.length() == 0)
    {
      docid_hash_.reserve(ENTRY_SIZE);
      for (size_t i=0; i<ENTRY_SIZE; i++)
        docid_hash_.add_tail(NULL);
    }

    IASSERT(docid_hash_.length() == ENTRY_SIZE);
  }

  /**
     @brief initialize hash table
   */
  inline void init_docid_hash()
  {
    boost::mutex::scoped_lock lock(mtx_);
    
    reset_docid_hash();
    
    for(size_t i=0; i<fp_list_->doc_num(); i++)
    {
      uint32_t docid = (*fp_list_)[i];
      size_t j = docid%ENTRY_SIZE;
      
      if (docid_hash_.at(j) == NULL)
        docid_hash_[j] = new Vector32();
      
      docid_hash_.at(j)->push_back(docid);
    }
/*
    {
      uint32_t docid = -1;
      size_t j = docid%ENTRY_SIZE;
      
      if (docid_hash_.at(j) == NULL)
        docid_hash_[j] = new Vector32();
      
      docid_hash_.at(j)->push_back(docid);
    }*/
  }

  inline void save_docid_hash()
  {
    boost::mutex::scoped_lock lock(mtx_);
    
    std::string s = filenm_;
    s+= ".docid_hash";

    FILE* f = fopen(s.c_str(), "w+");

    for (size_t i=0; i<ENTRY_SIZE; i++)
      if (docid_hash_.at(i) != NULL)
      {
        IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
        uint32_t len = docid_hash_.at(i)->length();
        IASSERT(fwrite(&len, sizeof(uint32_t), 1, f)==1);
        IASSERT(fwrite(docid_hash_.at(i)->data(), sizeof(uint32_t)*len, 1, f)==1);
      }

    uint32_t i = -1;
    IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
    fclose(f);
  }

  inline void load_docid_hash()
  {
    boost::mutex::scoped_lock lock(mtx_);
    
    std::string s = filenm_;
    s+= ".docid_hash";

    reset_docid_hash();
    
    FILE* f = fopen(s.c_str(), "r");
    if (f == NULL)
      return;

    uint32_t i = -1;
    IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    while (i!=(uint32_t)-1)
    {
      docid_hash_[i] = new Vector32();
      uint32_t len = 0;
      IASSERT(fread(&len, sizeof(uint32_t), 1, f)==1);
      IASSERT(fread(docid_hash_[i]->array(len), sizeof(uint32_t)*len, 1, f)==1);
      IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    }

    fclose(f);
  }

  /**
     @brief it generates prime number.
     @param s the amount of prime numbers need to be generated.
     @param v store all the generated prime number.
   */
  void prime_gen(uint32_t s, Vector32& v)
  {
    for (uint32_t i = 3; v.length()<s; ++i)
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

  
public:
  inline FpIndex(const char* filenm, uint8_t unit_len=2)
    : UNIT_LEN_(unit_len)
  {
    filenm_ = filenm;
    
    std::string s = filenm_;
    s+= ".final_group";
    final_group_ = new GroupT(s.c_str());
    
    fp_list_ = NULL;
    group_ = NULL;

    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      fp_hash_ptrs_[i] = NULL;

    load_docid_hash();
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

    if (final_group_!=NULL)
    {
      final_group_->flush();
      delete final_group_;
    }

    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      if (fp_hash_ptrs_[i]!= NULL)
    {
      fp_hash_ptrs_[i]->flush();
      delete fp_hash_ptrs_[i];
    }

    for (uint32_t i=0 ;i<docid_hash_.length(); ++i)
      if (docid_hash_.at(i) != NULL)
      {
        delete docid_hash_.at(i);
        docid_hash_[i] = NULL;
      }
  }

  inline void flush()
  {
    if (fp_list_!= NULL)
    {
      fp_list_->flush();
      delete fp_list_;
      fp_list_ = NULL;
    }
    
    if (group_!=NULL)
    {
      group_->flush();
      delete group_;
      group_ = NULL;
    }
    
    for (uint8_t i=0; i<FP_HASH_NUM; i++)
      if (fp_hash_ptrs_[i]!= NULL)
      {
        fp_hash_ptrs_[i]->flush();
        delete fp_hash_ptrs_[i];
        fp_hash_ptrs_[i] = NULL;
      }

    save_docid_hash();
  }

  /**
     @brief it must be called before insert FPs.
   */
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

    if (!fp_list_)
    {
      s = filenm_;
      s += ".fplist";
      fp_list_ = new FpList(s.c_str(), UNIT_LEN_);
    }

    if (!group_)
    {
      s = filenm_;
      s+= ".group";
      group_ = new GroupT(s.c_str());
    }

  }

  inline void ready_for_find()
  {
  }
  
  inline void add_doc(size_t docid, const izenelib::am::IntegerDynArray<UNIT_TYPE>& fp)
  {
    fp_list_->add_doc(docid, fp);
    
    docid = fp_list_->doc_num()-1;
    
    fp_hash_ptrs_[docid%FP_HASH_NUM]->add_doc(docid/FP_HASH_NUM, fp);
  }

  inline void update_doc(size_t docid, const izenelib::am::IntegerDynArray<UNIT_TYPE>& fp)
  {
    for (uint32_t j=0; j<fp_list_->doc_num(); ++j)
    {
      if ((*fp_list_)[j] == docid)
        (*fp_list_)[j] = -1;
    }

    add_doc(docid, fp);
  }
  
  void update_docs(const std::vector<uint32_t>& docids, const std::vector<
                   izenelib::am::IntegerDynArray<UNIT_TYPE> >& fps)
  {
    ready_for_insert();
    uint32_t num = fp_list_->doc_num();
    
    for (uint32_t i=0; i<docids.size(); ++i)
    {
      for (uint32_t j=0; j<fp_list_->doc_num(); ++j)
      {
        if ((*fp_list_)[j] == docids[i])
          (*fp_list_)[j] = -1;
      }

      add_doc(docids[i], fps[i]);
    }

    indexing(num);
    flush();
  }

  void del_doc(uint32_t docid)
  {
    for (uint32_t j=0; j<fp_list_->doc_num(); ++j)
    {
      if ((*fp_list_)[j] == docid)
        (*fp_list_)[j] = -1;
    }
  }
  
  void del_docs(const std::vector<uint32_t>& docids)
  {
    ready_for_insert();
    //uint32_t num = fp_list_->doc_num();
    
    for (uint32_t i=0; i<docids.size(); ++i)
    {
      for (uint32_t j=0; j<fp_list_->doc_num(); ++j)
      {
        if ((*fp_list_)[j] == docids[i])
          (*fp_list_)[j] = -1;
      }
    }

    switch_docid_in_group();
    init_docid_hash();
    flush();
  }
  

//   inline void indexing()
//   {
//     for (uint8_t i=0; i<FP_HASH_NUM; i++)
//       if (fp_hash_ptrs_[i]!= NULL)
//     {
//       fp_hash_ptrs_[i]->flush();
//       delete fp_hash_ptrs_[i];
//       fp_hash_ptrs_[i] = NULL;
//     }

//     ht_ptrs_ = new HashT*[FP_LENGTH/UNIT_LEN_];
//     for (uint8_t i=0; i<FP_LENGTH/UNIT_LEN_; i++)
//     {
//       std::string s = filenm_;
//       s += ".hasht";
//       s += i+'0';
//       ht_ptrs_[i] = new HashT(s.c_str());
//     }

//     for (uint8_t i=0; i<FP_LENGTH/UNIT_LEN_; i++)
//     {
//       fp_list_->ready_for_uniform_access(i);
//       for (size_t j=0; j<fp_list_->doc_num();j++)
//       {
//         uint64_t v = 0;
//         for (uint8_t k=0; k<UNIT_LEN_; k++)
//           v  = 37*v + fp_list_->get_fp(i, j, k);
    
//         ht_ptrs_[i]->push_back(v);
//       }
//       ht_ptrs_[i]->compact();
//     }

//     fp_list_->free();
//     std::cout<<"\nPre-clustering is over!(1.4GB)\n";
//     getchar();

//     //----------------------------------------
    
//     std::string s = filenm_;
//     s+= ".fphash";
//     for (uint8_t i=0; i<FP_HASH_NUM; i++)
//     {
//       std::string ss = s;
//       ss += '.';
//       ss += i+'0';
//       if (fp_hash_ptrs_[i]==NULL)
//         fp_hash_ptrs_[i] = new FpHashT(ss.c_str());
//       fp_hash_ptrs_[i]->ready_for_find();
//     }

//     Vector8 flags;
//     for (uint32_t i=0; i<fp_list_->doc_num();i++)
//     {
//       uint32_t docid1 = i;//fps[i];
//       memset(flags.array(fp_list_->doc_num()), fp_list_->doc_num(), 0);
//       const uint64_t* p1 = fp_hash_ptrs_[docid1%FP_HASH_NUM]->find(docid1/FP_HASH_NUM);

//       for (uint8_t k=0; k<FP_LENGTH/UNIT_LEN_; k++)
//       {
//         const Vector32& v = (*(ht_ptrs_[k]))[i];
      
//         for (size_t j = 0; j<v.length(); j++)
//         {
//           size_t index = v.at(j);
//           if (v.at(j) <= i)
//             continue;

//           uint32_t docid2 = index;//fps[index];
//           if (flags.at(docid2)==1 || group_->same_group(docid1, docid2))
//             continue;

//           const uint64_t* p2 = fp_hash_ptrs_[docid2%FP_HASH_NUM]->find(docid2/FP_HASH_NUM);
//           if (p1!=NULL && p2!=NULL && border_compare(p1,p2))
//           {
//             group_->add_doc(docid1, docid2);
//             //std::cout<<docid1<<" "<<docid2<<std::endl;
//           }
          
//           flags[docid2] = 1;
//         }
//       }

//       if (i%1000000==0)
//         std::cout<<(double)i/fp_list_->doc_num()*100.<<"% ...\n";
//     }

//     group_->compact();
//     //std::cout<<*group_<<std::endl;
//   }


  /**
     @brief group all the docs by their FPs, including pre-clustring.
     @param start the index of docs starting from which need to be grouped.
   */
  inline void indexing(uint32_t start = 0)
  {
    if (fp_list_==NULL || fp_list_->doc_num() == 0)
      return;
    
    struct timeval tvafter,tvpre;
    struct timezone tz;

    gettimeofday (&tvpre , &tz);
    
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
      for (size_t j=start; j<fp_list_->doc_num();j++)
      {
        UNIT_TYPE v = 0;
        for (uint8_t k=0; k<UNIT_LEN_; k++)
          v  = 37*v + fp_list_->get_fp(i, j, k);
    
        ht_ptrs_[i]->push_back(v);
      }
      ht_ptrs_[i]->compact();
      ht_ptrs_[i]->flush();
    }

    size_t doc_num = fp_list_->doc_num();
    delete fp_list_;fp_list_=NULL;//fp_list_->free();

    gettimeofday (&tvafter , &tz);
    std::cout<<"\nPre-clustering is over!("<<doc_num<<"): "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";

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
    
    Vector32 prime(doc_num+start);
    prime_gen(doc_num+start, prime);
    IASSERT(prime.length() == doc_num+start);
    Vector32 primeO = prime;
    
//     for (size_t i=0; i<doc_num; i++)
//     {
//       prime.add_tail(prime_->next());//std::cout<<prime[i]<<std::endl;
//     }
    
    //prime.add_tail(3);

    gettimeofday (&tvpre , &tz);
    std::cout<<"Start clustering...\n";
    uint32_t pairs = 0;
    size_t dealed = 0;
    for (size_t i=start; i<doc_num;i++)
    {
      for (uint8_t k=0; k<FP_LENGTH/UNIT_LEN_; k++)
      {
        const Vector32& v = (*(ht_ptrs_[k]))[i];
        //std::cout<<v<<std::endl;
        
        IASSERT(v.at(0)<=i);
        
        if (start == 0 && v.at(0)!= i)
          continue;

        dealed += v.length();
        for (size_t j = 0; j<v.length(); j++)
        {
          uint32_t docid1 = v.at(j);//fps[i];
          const UNIT_TYPE* p1 = fp_hash_ptrs_[docid1%FP_HASH_NUM]->find(docid1/FP_HASH_NUM);
          bool f=  false;

          for (size_t h=j+1; h<v.length(); h++)
          {
            size_t index = v.at(h);
            uint32_t docid2 = index;//fps[index];

            //if (prime.at(docid1)%(*prime_)[docid2]==0)
            if (prime.at(docid1)%primeO.at(docid2)==0)
            {
              f = true;
              continue;
            }
            
//             prime[docid1] *= (*prime_)[docid2];
//             prime[docid2] *= (*prime_)[docid1];
            
            prime[docid1] *= primeO.at(docid2);
            prime[docid2] *= primeO.at(docid1);

            const UNIT_TYPE* p2 = fp_hash_ptrs_[docid2%FP_HASH_NUM]->find(docid2/FP_HASH_NUM);

//             std::cout<<docid1<<" "<<(*p1)<<std::endl;
//             std::cout<<docid2<<" "<<(*p2)<<std::endl;
//             std::cout<<"-----------\n";
            
            if (p1!=NULL && p2!=NULL && charick_compare(p1,p2))
            {
              group_->add_doc(docid1, docid2);
              ++pairs;
            }
          }
          
          if (f)
            --dealed;
        }
       }

      if (dealed%10000==0)
      {
        gettimeofday (&tvafter , &tz);
        //std::cout<<(double)dealed/doc_num*100.<<"%: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";
      }
    }

    group_->compact();
    group_->flush();
    gettimeofday (&tvafter , &tz);
    std::cout<<"\nClustering is over!: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<" min\n";
    //std::cout<<*group_<<std::endl;

    std::cout<<"Detected DUP pairs: "<<pairs<<"\n";
    switch_docid_in_group();
    init_docid_hash();
    
    for (uint8_t i=0; i<FP_LENGTH/UNIT_LEN_; i++)
    {
      delete ht_ptrs_[i];
    }
    

    delete ht_ptrs_;
    ht_ptrs_ = NULL;
  }

  /**
     @return true if the doc in the collcection.
   */
  inline bool exist(uint32_t docid)
  {
    boost::mutex::scoped_lock lock(mtx_);
    
    IASSERT(docid_hash_.length()==ENTRY_SIZE);
    
    Vector32* v = docid_hash_.at(docid%ENTRY_SIZE);
    if (v == NULL)
      return false;

    for (size_t i=0; i<v->length(); i++)
      if (v->at(i) == docid)
        return true;
    return false;
  }

  /**
     @return vector of docids who are in the same group. 
   */
  inline Vector32 find(size_t docid)const
  {
    boost::mutex::scoped_lock lock(mtx_);
    Vector32 v = final_group_->find(docid);

    uint32_t i = v.find(-1);
    while (i!= (Vector32::size_t)-1)
    {
      v.erase(i);
      i = v.find(-1);
    }

    return v;
  }
  
  inline size_t doc_num()const
  {
    return fp_list_->doc_num();
  }

  /**
     @return true if 2 docs are duplicated by each other.
   */
  inline bool is_duplicated(uint32_t docid1, uint32_t docid2)const
  {
    boost::mutex::scoped_lock lock(mtx_);
    return final_group_->same_group(docid1, docid2);
  }
}
  ;


// template <
//   uint8_t  FP_HASH_NUM,
//   class    UNIT_TYPE,
//   uint8_t  FP_LENGTH,
//   uint32_t CACHE_SIZE,
//   uint32_t ENTRY_SIZE
//   >
// izenelib::ir::PrimeGen<>* FpIndex<FP_HASH_NUM,UNIT_TYPE,FP_LENGTH,
//                                  CACHE_SIZE,ENTRY_SIZE>::prime_ =  new izenelib::ir::PrimeGen<>("./prime_num");

NS_IZENELIB_IR_END

#endif

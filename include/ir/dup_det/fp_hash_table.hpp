#ifndef FP_HASH_TABLE_HPP
#define FP_HASH_TABLE_HPP

#include <types.h>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <string>

NS_IZENELIB_IR_BEGIN

template <
  uint32_t CACHE_SIZE = 600,//MB
  uint8_t  FP_LENGTH = 48,//bytes
  uint32_t ENTRY_SIZE = 1000000
  >
class FpHashTable
{
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<uint64_t> Vector64;
  typedef izenelib::am::IntegerDynArray<Vector32*> Vector32Ptr;
  typedef Vector32::size_t size_t;
  typedef FpHashTable<CACHE_SIZE, FP_LENGTH, ENTRY_SIZE> SelfT;
  
protected:
  const uint32_t NUM_IN_MEM_;
  Vector32Ptr entry_;
  FILE* hash_f_;
  size_t doc_num_;

  //----------cache------
  uint32_t swich_p_;
  Vector32 keys_;
  Vector64 fps_;
  FILE* fp_f_;
  FILE* key_f_;
  
  

  inline void load_cache(size_t i=0)
  {
    //-------------------Load cache-----------------------
    uint32_t size = doc_num_-i>NUM_IN_MEM_? NUM_IN_MEM_: doc_num_-i;
    fseek(fp_f_, i*FP_LENGTH, SEEK_SET);
    assert(fread(fps_.array(size*FP_LENGTH/sizeof(uint64_t)), size*FP_LENGTH, 1, fp_f_)==1 );
    
    fseek(key_f_, i*sizeof(uint32_t), SEEK_SET);
    assert(fread(keys_.array(size),size*sizeof(uint32_t), 1, key_f_)==1 );
  }
  
  inline void load_hash()
  {
    //-----------Load doc hash table-----------
    fseek(hash_f_, 0, SEEK_SET);
    assert (fread(&doc_num_, sizeof(size_t), 1,hash_f_)==1);
    if (doc_num_ == 0)
      return;

    size_t index = 0;
    assert(fread(&index, sizeof(size_t), 1, hash_f_)==1);
    for (size_t i=0; i<entry_.length(); i++)
    {
      if (index != i)
        continue;

      size_t len = 0;
      assert(fread(&len, sizeof(size_t), 1, hash_f_)==1);
      entry_[i] = new Vector32();
      assert(fread(entry_.at(i)->array(len), len*sizeof(uint32_t), 1, hash_f_)==1);
      if (fread(&index, sizeof(size_t), 1, hash_f_)!=1)
      {
        //std::cout<<*this<<std::endl;
        return;
      }
    }
  }


  /**
     Used for adding docs.
   **/
  inline void unload_cache()
  {
    if (keys_.length()== 0)
      return;
    
    fseek(fp_f_, swich_p_*FP_LENGTH, SEEK_SET);
    assert( fwrite(fps_.data(), fps_.size(), 1, fp_f_)==1);

    fseek(key_f_, swich_p_*sizeof(uint32_t), SEEK_SET);
    assert(fwrite(keys_.data(), keys_.size(), 1, key_f_)==1);
    
    swich_p_ = doc_num_;
    keys_.compact();
    keys_.reset();

    fps_.compact();
    fps_.reset();
  }

  inline void unload_hash()
  {
    fseek(hash_f_, 0, SEEK_SET);
    assert( fwrite(&doc_num_, sizeof(size_t), 1, hash_f_)==1);

    for (size_t i=0; i<entry_.length(); i++)
    {
      Vector32* v = entry_.at(i);
      if (v==NULL)
        continue;

      assert( fwrite(&i, sizeof(size_t), 1, hash_f_)==1);
      size_t len = v->length();
      assert( fwrite(&len, sizeof(size_t), 1, hash_f_)==1);
      assert( fwrite(v->data(), v->size(), 1, hash_f_)==1);
    }

  }

  inline void set_addr(size_t docid, size_t addr)
  {
    size_t i = docid%ENTRY_SIZE;
    assert (entry_.at(i)!=NULL);

    Vector32* p  = entry_.at(i);
    
    for (i = 0; i<p->length(); i++)
    {
      if (p->at(i++) == docid)
      {
        (*p)[i] = addr;
        return;
      }
    }
  }

  inline size_t cache_switch(size_t docid)
  {
    fseek(key_f_, docid*sizeof(uint32_t), SEEK_SET);
    assert( fread(&keys_[swich_p_], sizeof(uint32_t), 1, key_f_)==1);

    fseek(fp_f_, docid*FP_LENGTH, SEEK_SET);
    assert( fread(&fps_[swich_p_*FP_LENGTH/sizeof(uint64_t)], FP_LENGTH, 1, fp_f_)==1);

    size_t r = swich_p_;
    
    if (swich_p_!=0)
      --swich_p_;
    else
      swich_p_  = keys_.length()-1;

    return r;
  }
  
public:
  inline FpHashTable(const char* filenm)
    : NUM_IN_MEM_(CACHE_SIZE*1000000/(FP_LENGTH+sizeof(uint32_t)))
  {
    doc_num_ = 0;
    entry_.reserve(ENTRY_SIZE);
    for (uint32_t i=0; i<ENTRY_SIZE; i++)
        entry_.add_tail(NULL);

    std::string nm = filenm;
    nm += ".has";
    hash_f_ = fopen(nm.c_str(), "r+");
    if (hash_f_ == NULL)
    {
      hash_f_ = fopen(nm.c_str(), "w+");
      if (hash_f_ == NULL)
      {
        std::cout<<"Can't create file: "<<nm<<std::endl;
        return;
      }
      swich_p_ = 0;
      assert( fwrite(&doc_num_, sizeof(size_t), 1, hash_f_)==1);
    }
    else
      load_hash();

    nm = filenm;
    nm += ".key";
    key_f_ = fopen(nm.c_str(), "r+");
    if (key_f_ == NULL)
    {
      key_f_ = fopen(nm.c_str(), "w+");
      if (key_f_ == NULL)
      {
        std::cout<<"Can't create file: "<<nm<<std::endl;
        return;
      }
    }

    nm = filenm;
    nm += ".fp";
    fp_f_ = fopen(nm.c_str(), "r+");
    if (fp_f_ == NULL)
    {
      fp_f_ = fopen(nm.c_str(), "w+");
      if (fp_f_ == NULL)
      {
        std::cout<<"Can't create file: "<<nm<<std::endl;
        return;
      }
    }

    swich_p_ = doc_num_;
  }

  inline ~FpHashTable()
  {
    for (size_t i=0; i<entry_.length(); i++)
    {
      Vector32* v = entry_.at(i);
      if (v==NULL)
        continue;
      delete v;
    }

    fclose(fp_f_);
    fclose(key_f_);
    fclose(hash_f_);
  }

  inline void flush()
  {
    unload_cache();
    unload_hash();
    
    fflush(fp_f_);
    fflush(hash_f_);
    fflush(key_f_);
  }

  inline void ready_for_insert()
  {
    load_hash();
    swich_p_ = doc_num_;
    
    keys_.reset();
    fps_.reset();
    swich_p_ = doc_num_;
  }

  inline void ready_for_find()
  {
    if (swich_p_ != doc_num_)
    {
      unload_cache();
      unload_hash();
    }
    
    keys_.reset();
    fps_.reset();

    load_cache();
    swich_p_ = keys_.length()-1;
  }
  
  inline void add_doc(size_t docid, const Vector64& fp)
  {
    if (keys_.length()>=NUM_IN_MEM_)
    {
      unload_cache();
    }

    keys_.push_back(docid);
    fps_ += fp;

    size_t i = docid%ENTRY_SIZE;
    if (entry_.at(i)==NULL)
      entry_[i] = new Vector32(2);

    entry_.at(i)->push_back(docid);
    entry_.at(i)->push_back(docid);
    ++doc_num_;
  }

  inline const uint64_t* find(size_t docid)
  {
    static uint32_t total = 0;
    ++total;
    static uint32_t lose = 0;
    
    size_t i = docid%ENTRY_SIZE;
    if (entry_.at(i)==NULL)
      return NULL;

    Vector32* p  = entry_.at(i);
    if (total!=0 && total%1000000==0)
      std::cout<<"cache lost rate: "<<(double)lose/total<<"\n";
    
    for (i = 0; i<p->length(); i++)
    {
      if (p->at(i++) == docid)
      {
        size_t k = p->at(i);
        if (k>=keys_.length() ||keys_.at(k)!=docid)
        {
          ++lose;
          k = (*p)[i] = cache_switch(docid);
          //std::cout<<"----------\n";
        }

        return fps_.data()+k*FP_LENGTH/sizeof(uint64_t);
      }
    }

    return NULL;
  }
  
  inline size_t doc_num()const
  {
    return doc_num_;
  }


  inline void compact()
  {
    for (size_t i=0; i<entry_.length(); i++)
    {
      Vector32* v = entry_.at(i);
      if (v==NULL)
        continue;
      v->compact();
    }

    keys_.compact();
    fps_.compact();
  }
  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const SelfT& v)
  {
    os<<"\n------- hash table ("<<v.doc_num_<<")------\n";
    for (size_t i =0; i<v.entry_.length(); i++)
    {
      Vector32* p = v.entry_.at(i);
      if (p==NULL)
        continue;

      os<<"#"<<i<<": ";
      os<<(*p)<<std::endl;
    }

    os<<"--------- Cache  ---------\n";
    for (size_t i=0; i<v.keys_.length(); i++)
    {
      os<<v.keys_.at(i)<<"=>";
      for (size_t j=0; j<FP_LENGTH/sizeof(uint64_t); j++)
        os<< v.fps_.at(i*FP_LENGTH/sizeof(uint64_t)+j)<<" ";
      os<<"\n";
    }
    
    
    return os;
  }

  
}
  ;

NS_IZENELIB_IR_END
#endif

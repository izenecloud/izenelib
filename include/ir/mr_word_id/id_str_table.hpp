/**
   @file id_str_table.hpp
   @author Kevin Hu
   @date 2010.01.08
*/
#ifndef ID_STR_TABLE_HPP
#define ID_STR_TABLE_HPP

#include <types.h>

#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#include <util/hashFunction.h>
#include <am/graph_index/dyn_array.hpp>

NS_IZENELIB_IR_BEGIN
/**
 * @class TermFrequency
 * @brief TermFrequency is used for caculating the frequency of terms. It's a string hash table essentially.
 *
 **/

template<
  typename IDType = uint32_t,
  typename LenType = uint8_t,
  uint8_t ENTRY_POW = 17
  >
class IdStringTable
{
  enum {ENTRY_SIZE = (2<<ENTRY_POW)};
  enum {ENTRY_MASK = ENTRY_SIZE-1};
  enum 
    {
      INIT_BUCKET_SIZE = 64
    };

  typedef uint32_t PosType;

  struct ID_STRUCT
  {
    char id[sizeof(IDType)];
    char pos[sizeof(PosType)];

    IDType& ID_()
    {
      return *(IDType*)id;
    }

    PosType& POS_()
    {
      return *(PosType*)pos;
    }

    IDType ID()const
    {
      return *(IDType*)id;
    }

    PosType POS()const
    {
      return *(PosType*)pos;
    }
  
    inline ID_STRUCT(IDType i, PosType j)
    {
      ID_() = i;
      POS_() = j;
    }
  
    inline ID_STRUCT(IDType i)
    {
      ID_() = i;
      POS_() = 0;
    }
  
    inline ID_STRUCT()
    {
      ID_() = 0;
      POS_() = -1;
    }
  
    inline ID_STRUCT(const ID_STRUCT& other)
    {
      ID_() = other.ID();
      POS_() = other.POS();
    }

    inline ID_STRUCT& operator = (const ID_STRUCT& other)
    {
      ID_() = other.ID();
      POS_() = other.POS();
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
      return (ID() < other.ID());
    }

    inline bool operator > (const ID_STRUCT& other)const
    {
      return (ID() > other.ID());
    }

    inline bool operator <= (const ID_STRUCT& other)const 
    {
      return (ID() <= other.ID());
    }

    inline bool operator >= (const ID_STRUCT& other)const
    {
      return (ID() >= other.ID());
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
      os<<"<"<<v.ID()<<","<<v.POS()<<">";

      return os;
    }
  
  }
    ;

  typedef izenelib::am::DynArray<ID_STRUCT> bucket_t;

#define BS(buf) (*(uint32_t*)(buf))//buffer size
#define BCS(buf) (*(uint32_t*)((uint32_t*)(buf)+1))//buffer content size
#define LEN(buf) (*(LenType*)(buf))
#define ENT(i) ((bucket_t*)(entry_[i])) //entry[i]
  
protected:
  char** entry_;//!< entry of table
  char* str_buf_;
  
  uint32_t count_;

  void clean_()
  {
    for(uint32_t i=0; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]!= NULL)
        delete (entry_[i]);
      
      entry_[i] = NULL;
    }

    count_ = 0;
    if (str_buf_!=NULL)
      free(str_buf_);

    str_buf_ =NULL;
  }
  
public:
  /**
     @brief a constructor
  */
  IdStringTable()
  {
    entry_ = new char*[ENTRY_SIZE];
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      entry_[i] = NULL;
    }
    count_ = 0;
    str_buf_ = NULL;
  }
    
  /**
     @brief a destructor
  */
  ~IdStringTable()
  {
    clean_();
    delete entry_;
  }

  uint32_t num_items() const
  {
    return count_;
  }

  void display(std::ostream & os=std::cout)
  {
    for(size_t i=0; i<ENTRY_SIZE; i++)
    {
      bucket_t* pBkt = (bucket_t*)entry_[i];

      if (pBkt ==NULL)
        continue;

      os<<endl<<i<<" Entry===========\n";
      os<<(*pBkt)<<std::endl;
    }
  
  }

  bool insert(IDType id, LenType len, const char* buf)
  {
    uint32_t tmp;
    if (find(id, tmp) != NULL)
      return false;

    if (str_buf_ == NULL)
    {
      str_buf_ = (char*)malloc(INIT_BUCKET_SIZE);
      BS(str_buf_) = INIT_BUCKET_SIZE;
      BCS(str_buf_) = 2*sizeof(uint32_t);
    }

    uint32_t cs = BCS(str_buf_);
    while (len + sizeof(LenType)+cs> BS(str_buf_))
    {
      str_buf_ = (char*)realloc(str_buf_, BS(str_buf_) + INIT_BUCKET_SIZE);
      BS(str_buf_) += INIT_BUCKET_SIZE;
    }

    ID_STRUCT ids(id, cs);
    uint32_t idx  = ids & ENTRY_MASK;
    if (entry_[idx] == NULL)
      entry_[idx] = (char*)(new bucket_t());
    ENT(idx)->push_back(ids);
    
    LEN(str_buf_+cs) = len;
    cs += sizeof(LenType);
    memcpy(str_buf_+cs, buf, len);
    BCS(str_buf_) = cs + len;

    ++count_;
    return true;
  }
  
  bool update(IDType id, LenType len, const char* buf)
  {
    uint32_t tmp;
    if (find(id, tmp) == NULL)
      return false;

    IASSERT(str_buf_ != NULL);

    uint32_t cs = BCS(str_buf_);
    while (len + sizeof(LenType)+cs> BS(str_buf_))
    {
      str_buf_ = (char*)realloc(str_buf_, BS(str_buf_) + INIT_BUCKET_SIZE);
      BS(str_buf_) += INIT_BUCKET_SIZE;
    }

    ID_STRUCT ids(id, cs);
    uint32_t idx  = ids & ENTRY_MASK;
    IASSERT (entry_[idx] != NULL);
    
    typename bucket_t::size_t i = ENT(idx)->find(ids);
    IASSERT(i!=bucket_t::NOT_FOUND);
    (*ENT(idx))[i] = ids;
    
    LEN(str_buf_+cs) = len;
    cs += sizeof(LenType);
    memcpy(str_buf_+cs, buf, len);
    BCS(str_buf_) = cs + len;

    return true;
  }

  bool remove(IDType id)
  {
    ID_STRUCT ids(id);
    uint32_t idx  = ids & ENTRY_MASK;
    if (entry_[idx] == NULL)
      return false;

    typename bucket_t::size_t i = ENT(idx)->find(ids);
    if(i == bucket_t::NOT_FOUND)
      return false;

    ENT(idx)->erase(i);
    --count_;
    return true;
  }
  
  const char* find(IDType id, uint32_t& len)const
  {
    len = 0;
    ID_STRUCT ids(id);

    uint32_t idx  = ids & ENTRY_MASK;
    if (entry_[idx] == NULL)
      return NULL;

    typename bucket_t::size_t i = ENT(idx)->find(ids);
    if(i == bucket_t::NOT_FOUND)
      return NULL;

    IASSERT(str_buf_!=NULL);
    IASSERT(BS(str_buf_)>=BCS(str_buf_));
    IASSERT((*ENT(idx))[i].POS()<BCS(str_buf_));

    len = LEN(str_buf_ + (*ENT(idx))[i].POS());
    
    return str_buf_ + (*ENT(idx))[i].POS() + sizeof(LenType);
  }
  
  void save(FILE* f)const
  {
    fseek(f, 0, SEEK_SET);
    IASSERT(fwrite(&count_, sizeof(uint32_t), 1, f)==1);
    
    uint32_t i=0;
    for(; i<ENTRY_SIZE; i++)
    {
      if (entry_[i]==NULL)
        continue;

      IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);
      ENT(i)->save(f);
    }

    i=-1;
    IASSERT(fwrite(&i, sizeof(uint32_t), 1, f)==1);

    IASSERT(fwrite(str_buf_, BS(str_buf_), 1, f)==1);
  }
  
  void load(FILE* f)
  {
    clean_();
    
    uint32_t i=0;
    uint32_t bs = 0;
    
    fseek(f, 0, SEEK_SET);
    IASSERT(fread(&count_, sizeof(uint32_t), 1, f)==1);

    IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    while (i != (uint32_t)-1)
    {
      entry_[i] = (char*)(new bucket_t());
      ENT(i)->load(f);
      IASSERT(fread(&i, sizeof(uint32_t), 1, f)==1);
    }

    IASSERT(fread(&bs, sizeof(uint32_t), 1, f)==1);
    str_buf_ = (char*)malloc(bs);
    BS(str_buf_) = bs;
    IASSERT(fread(&BCS(str_buf_), bs - sizeof(uint32_t), 1, f)==1);
  }
  
};

NS_IZENELIB_IR_END

#endif

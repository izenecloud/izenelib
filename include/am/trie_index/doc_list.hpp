#ifndef DOC_LIST_HPP
#define DOC_LIST_HPP

#include <types.h>
#include <vector>
#include <ostream>

NS_IZENELIB_AM_BEGIN

template<
  typename DOCID_TYPE = uint32_t
  >
class DocList
{
  typedef DocList<DOCID_TYPE> SelfT;
  
  struct LIST_HEAD
  {
    uint32_t max_num;
    uint32_t num;
  }
    ;

  char* buf_;

#define GET_MAX_NUM(buf) (((LIST_HEAD*)(buf))->max_num)
#define GET_NUM(buf) (((LIST_HEAD*)(buf))->num)
#define FIRST(buf)   (*(DOCID_TYPE*)(buf+sizeof(struct LIST_HEAD)))
#define END(buf)   (*(DOCID_TYPE*)(buf+sizeof(struct LIST_HEAD)+GET_NUM(buf)*sizeof(DOCID_TYPE)))
#define GET(buf, i)   (*(DOCID_TYPE*)(buf+sizeof(struct LIST_HEAD)+(i)*sizeof(DOCID_TYPE)))

public:
  DocList(DOCID_TYPE id)
  {
    buf_ = (char*)malloc(sizeof(struct LIST_HEAD)+sizeof(DOCID_TYPE));
    GET_MAX_NUM(buf_) = GET_NUM(buf_) = 1;
    FIRST(buf_) = id;
  }

  DocList(const SelfT& other)
  {
    buf_ = (char*)malloc(sizeof(struct LIST_HEAD)+other.length()*sizeof(DOCID_TYPE));
    GET_MAX_NUM(buf_) = GET_NUM(buf_) = other.length();
    memcpy(&FIRST(buf_), other.begin(), other.length()*sizeof(DOCID_TYPE));
  }

  DocList(FILE* f, uint64_t addr)
  {
    struct LIST_HEAD head;
    fseek(f, addr, SEEK_SET);
    assert(fread(&head, sizeof(struct LIST_HEAD),1,f)==1);
    
    buf_ = (char*)malloc(sizeof(struct LIST_HEAD)+head.num*sizeof(DOCID_TYPE));
    GET_MAX_NUM(buf_) = GET_NUM(buf_) = head.num;
    assert(fread(buf_+sizeof(struct LIST_HEAD), GET_NUM(buf_)*sizeof(DOCID_TYPE),1, f)==1);
  }

  ~DocList()
  {
    free(buf_);
  }

  inline uint32_t length()const
  {
    return GET_NUM(buf_);
  }

  inline uint32_t size()const
  {
    return GET_MAX_NUM(buf_)*sizeof(DOCID_TYPE)+sizeof(struct LIST_HEAD);
  }

  inline const char* begin()const
  {
    return (char*)&FIRST(buf_);
  }
  
  void append(DOCID_TYPE id)
  {
    if (END(buf_) == id)
      return;
    
    if (GET_MAX_NUM(buf_) - GET_NUM(buf_)==0)
    {
      buf_ = (char*)realloc(buf_, 2*GET_NUM(buf_)*sizeof(DOCID_TYPE)+sizeof(struct LIST_HEAD));
      GET_MAX_NUM(buf_) = 2*GET_NUM(buf_);
    }

    END(buf_) = id;
    GET_NUM(buf_)++;
  }

  void append(const SelfT& other)
  {
    DOCID_TYPE end = (*this)[length()-1];//GET(buf_, length()-1);
    uint32_t i = 0;
    
    while (i<other.length() && *((DOCID_TYPE*)other.begin()+i)==end)
      ++i;
    
    
    if (GET_MAX_NUM(buf_) - GET_NUM(buf_) < other.length())
    {
      buf_ = (char*)realloc(buf_, (GET_NUM(buf_)+other.length()-i)*sizeof(DOCID_TYPE)+sizeof(struct LIST_HEAD));
      GET_MAX_NUM(buf_) = GET_NUM(buf_)+other.length()-i;
    }

    
    memcpy((char*)&END(buf_), other.begin()+i*sizeof(DOCID_TYPE), (other.length()-i)*sizeof(DOCID_TYPE));
    
    GET_NUM(buf_)+=(other.length()-i);
  }

  DOCID_TYPE operator [](uint32_t i)const
  {
    assert(i< GET_NUM(buf_));
    return GET(buf_, i);
  }

  inline void compact()
  {
    if (GET_MAX_NUM(buf_) - GET_NUM(buf_) == 0)
      return;
    
    buf_ = (char*)realloc(buf_, GET_NUM(buf_)*sizeof(DOCID_TYPE)+sizeof(struct LIST_HEAD));
    GET_MAX_NUM(buf_) = GET_NUM(buf_);
  }

  void save(FILE* f)const
  {
    uint32_t max = GET_MAX_NUM(buf_);
    GET_MAX_NUM(buf_) = GET_NUM(buf_);
    assert(fwrite(buf_, GET_NUM(buf_)*sizeof(DOCID_TYPE)+sizeof(struct LIST_HEAD),1, f)==1);
    GET_MAX_NUM(buf_) = max;
  }

  inline void assign(std::vector<DOCID_TYPE>& v)
  {
    v.clear();
    
    v.resize(length());
    memcpy(v.data(), &FIRST(buf_), length()*sizeof(DOCID_TYPE));
  }

friend std::ostream& operator<< (std::ostream& os, const SelfT& list)
  {
    for (uint32_t i=0; i<list.length(); ++i)
      os<<list[i]<<" ";
    return os;
  }
}
  ;

NS_IZENELIB_AM_END
#endif

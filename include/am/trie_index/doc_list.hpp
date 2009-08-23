#ifndef DOC_LIST_HPP
#define DOC_LIST_HPP

#include <types.h>

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

#define GET_MAX_NUM(buf) (((LIST_HEAD*)(buf))->max_size)
#define GET_NUM(buf) (((LIST_HEAD*)(buf))->size)
#define FIRST(buf)   (*(DOCID_TYPE*)(buf+sizeof(struct LIST_HEAD)))
#define END(buf)   (*(DOCID_TYPE*)(buf+sizeof(struct LIST_HEAD)+GET_NUM(buf)*sizeof(DOCID_TYPE)))
#define GET(buf, i)   (*(DOCID_TYPE*)(buf+sizeof(struct LIST_HEAD)+i*sizeof(DOCID_TYPE)))

public:
  DocList(DOCID_TYPE id)
  {
    buf_ = (char*)malloc(sizeof(struct LIST_HEAD)+sizeof(DOCID_TYPE));
    GET_MAX_NUM(buf_) = GET_NUM(buf_) = 1;
    FIRST(buf_) = id;
  }

  DocList(FILE* f, uint64_t addr)
  {
    uint32_t len = 0;
    fseek(f, addr+sizeof(uint32_t), SEEK_SET);
    assert(fread(&len, sizeof(uint32_t),1,f)==1);
    buf_ = (char*)malloc(sizeof(struct LIST_HEAD)+len*sizeof(DOCID_TYPE));
    GET_MAX_NUM(buf_) = len;
    GET_NUM(buf_) = len;
    assert(fread(buf_, GET_NUM(buf_)*sizeof(DOCID_TYPE),1, f)==1);
  }

  ~DocList()
  {
    free(buf_);
  }

  inline uint32_t length()const
  {
    return GET_NUM(buf_);
  }

  inline const char* begin()const
  {
    return (char*)FIRST(buf_);
  }
  
  void append(DOCID_TYPE id)
  {
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
    if (GET_MAX_NUM(buf_) - GET_NUM(buf_) < other.length())
    {
      buf_ = (char*)realloc(buf_, (GET_NUM(buf_)+other.length())*sizeof(DOCID_TYPE)+sizeof(struct LIST_HEAD));
      GET_MAX_NUM(buf_) = GET_NUM(buf_)+other.length();
    }

    memcpy((char*)END(buf_), other.begin(), other.length()*sizeof(DOCID_TYPE));
    
    GET_NUM(buf_)+=other.length();
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
}
  ;

NS_IZENELIB_AM_END
#endif

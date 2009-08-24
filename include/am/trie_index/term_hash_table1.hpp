#ifndef TERM_HASH_TABLE
#define TERM_HASH_TABLE

#include<types.h>
#include <ostream>
#include <assert.h>
#include <stdio.h>
#include <iostream>
//#include <hashfunction.h>

NS_IZENELIB_AM_BEGIN


/**
   len(1)|value(8)|term\0|....
 **/
class Term
{
public:
  typedef uint8_t len_t;

private:
  char* buf_;

  struct TERM_HEAD
  {
    len_t len_;
    len_t value_[8];
  }
    ;

public:
  inline explicit Term(char* buf)
    :buf_(buf)
  {
  }

  Term(const char* str, len_t len/*'\0' excluded*/, char* buf, uint64_t value)
  {
    buf_ = buf;
    ((TERM_HEAD*)buf_)->len_ = ++len;
    *(uint64_t*)((TERM_HEAD*)buf_)->value_ = value;

    buf = buf_+sizeof(struct TERM_HEAD);
    for (len_t i = 0; i<len;++i,++buf)
      *buf = str[i];

    *buf = '\0';

    //std::cout<<"--"<<*this<<std::endl;
  }

  static inline uint32_t need_size(uint8_t s)
  {
    return sizeof(struct TERM_HEAD)+s+1;
  }
  

  inline len_t length()const
  {
    return ((TERM_HEAD*)buf_)->len_-1;
  }

  inline uint32_t size()const
  {
    return ((TERM_HEAD*)buf_)->len_ + sizeof(struct TERM_HEAD);
  }

  inline uint64_t value()const
  {
    return *(uint64_t*)((TERM_HEAD*)buf_)->value_;
  }

  inline const char* term() const
  {
    return buf_+sizeof(struct TERM_HEAD);
  }

  inline char* next() const
  {
    return buf_ + sizeof(struct TERM_HEAD)+ ((TERM_HEAD*)buf_)->len_;
  }

  /**
     If it's not equal, return next term, if it is, return current term.
   **/
  inline bool is_equal(const char* str, len_t len, char** t)
  {
    if (((TERM_HEAD*)buf_)->len_-1!=len)
    {
      *t = next();
      return false;
    }

    char* buf = buf_ + sizeof(struct TERM_HEAD);
    
    for (len_t i=0; i<len; ++i, ++buf)
      if (*buf != str[i])
      {
        *t = (buf+len-i+1);
        return false;
      }

    *t = (buf_);
    return true;
  }

  bool operator != (const Term& other)
  {
    return buf_!=other.buf_;
  }
  
friend std::ostream& operator << (std::ostream& os, const Term& t)
  {
    os<<t.buf_+sizeof(struct Term::TERM_HEAD );
    return os;
  }
  
//   inline void add_freq()
//   {
//     *(freq_t*)(buf_+sizeof(len_t)) += 1;
//   }

//   inline void set_child(uint64_t child)
//   {
//     *(uint64_t*)(buf_+sizeof(len_t)+sizeof(freq_t)+1) = child;
//   }

//   inline void set_doc_list(uint64_t list)
//   {
//     *(uint64_t*)(buf_+sizeof(len_t)+sizeof(freq_t)+1+sizeof(uint64_t)) = list;
//   }
  
  
}  ;


/**
   max_size(4)|size(4)|index(4)|count(4)|Term...
   max_size is the size of the entire buffer ;
 **/
class Row
{
  struct ROW_HEAD
  {
    uint32_t max_size_;
    uint32_t size_;
    uint32_t index_;
    uint32_t count_;
  }
    ;
  
  char* buf_;
  bool  const_buf_;

public:
  typedef uint8_t len_t;
  
  inline Row(uint32_t size, uint32_t index)
  {
    size = Term::need_size(size) + sizeof(struct ROW_HEAD)+1;
    buf_ = (char*)malloc(size);
    const_buf_ = false;
    ROW_HEAD * h = (ROW_HEAD*)buf_;
    h->max_size_ = size;
    h->size_ = 4*sizeof(uint32_t);
    h->index_ = index;
    h->count_ = 0;
  }

  inline Row(FILE* f)
  {
    uint32_t size = 0;
    assert(fread(&size, sizeof(uint32_t), 1, f)==1);
    buf_ = (char*)malloc(size);
    ((ROW_HEAD*)buf_)->size_ = size;
    assert(fread(buf_+sizeof(uint32_t), size-sizeof(uint32_t), 1, f)==1);
    const_buf_ = false;
  }
  
  inline Row(char* buf)
  {
    buf_ = buf;
    const_buf_ = true;
  }

  inline ~Row()
  {
    if (!const_buf_)
      free(buf_);
  }

  inline const char* get_buf()const
  {
    return buf_;
  }

  inline Term begin()const
  {
    return Term(buf_ + sizeof(struct ROW_HEAD));
  }
  
  inline uint32_t max_size()const
  {
    return ((ROW_HEAD*)buf_)->max_size_;
  }

  inline uint32_t size() const
  {
    return ((ROW_HEAD*)buf_)->size_;
  }

  inline uint32_t count()const
  {
    return ((ROW_HEAD*)buf_)->count_;
  }

  inline void set_index(uint32_t i)
  {
    ((ROW_HEAD*)buf_)->index_ = i;
  }

  inline uint32_t index()const
  {
    return ((ROW_HEAD*)buf_)->index_;
  }
  
  inline void append_term(const char* str, len_t len, uint64_t value)
  {
    while (max_size()-size()<= Term::need_size(len))
    {
      buf_ = (char*)realloc(buf_, 2*size());
      ((ROW_HEAD*)buf_)->max_size_ = 2*size();
    }

    char* r = buf_ + size();
    Term t(str, len, r, value);
    ((ROW_HEAD*)buf_)->size_ += t.size();
    ((ROW_HEAD*)buf_)->count_++;
  }

  uint64_t add_term(const char* str, len_t len, uint64_t value)
  {
    char* p = buf_ + sizeof(struct ROW_HEAD);
    uint32_t num = count();
    
    for (uint32_t i=0; i<num; ++i)
    {
      Term t(p);
      if (t.is_equal(str, len, &p))
        return t.value();
    }

    while (max_size()-size()<= Term::need_size(len))
    {
      uint32_t g = p - buf_;
      buf_ = (char*)realloc(buf_, 2*size());
      ((ROW_HEAD*)buf_)->max_size_ = 2*size();
      p = buf_ + g;
    }
    
    Term t = Term(str, len, p, value);

    ((ROW_HEAD*)buf_)->size_ += t.size();
    ((ROW_HEAD*)buf_)->count_++;

    //std::cout<<"=="<<*this<<std::endl;

    return value;
    
  }

  uint64_t find(const char* str, len_t len)
  {
    char* p = buf_ + sizeof(struct ROW_HEAD);
    uint32_t num = count();
    
    for (uint32_t i=0; i<num; ++i)
    {
      Term t(p);
      if (t.is_equal(str, len, &p))
        return t.value();
    }

    return -1;
  }

  inline void compact()
  {
    uint32_t s = size();
    if (max_size()-s<=1)
      return;

    buf_ = (char*)realloc(buf_, s);
    ((ROW_HEAD*)buf_)->max_size_ = s;
  }

  inline uint64_t save(FILE* f)
  {
    uint32_t s = ((ROW_HEAD*)buf_)->max_size_;
    ((ROW_HEAD*)buf_)->max_size_ = ((ROW_HEAD*)buf_)->size_;
    
    assert(fwrite(buf_, ((ROW_HEAD*)buf_)->size_, 1, f)==1);
    ((ROW_HEAD*)buf_)->max_size_ = s;

    return ((ROW_HEAD*)buf_)->size_;
  }
  

friend std::ostream& operator << (std::ostream& os, const Row& row)
  {
    os<<"\nmax_size:"<<row.max_size()<<"|size:"<<row.size()<<"|index:"<<row.index()<<"|count:"<<row.count()<<"\n----------\n";

    Term t(row.buf_+sizeof(struct Row::ROW_HEAD));
    
    for (uint32_t i=0; i< row.count(); ++i)
    {
      os<<"|"<<t;
      t = Term(t.next());
    }

    os<<std::endl;
    return os;
  }
  
  
}
  ;

class TermHashTable
{
  uint32_t num_;
  Row**    entry_;
  uint32_t entry_size_;

  typedef TermHashTable SelfT;
  typedef uint8_t len_t;

public:
  class const_iterator
  {
    const Row**    entry_;
    const uint32_t entry_size_;
    uint32_t entry_num_;
    Term t_;

  public:
    const_iterator(const Row** entry, uint32_t entry_size, uint32_t entry_num)
      : entry_(entry), entry_size_ (entry_size), entry_num_(entry_num),
        t_(entry_num< entry_size? entry_[entry_num_]->begin(): Term(0))
    {
    }

    
    const_iterator(const const_iterator& other)
      : entry_(other.entry_), entry_size_ (other.entry_size_), entry_num_(other.entry_num_), t_(other.t_)
    {
    }

    const Term& operator *()const
    {
      return t_;
    }

    const_iterator& operator++()
    {
      //std::cout<<t_<<std::endl;
      char* p = t_.next();
      if (p - entry_[entry_num_]->get_buf()>=entry_[entry_num_]->size())
      {
        for (++entry_num_; entry_num_<entry_size_; ++entry_num_)
          if (entry_[entry_num_]!=NULL)
          {
            t_ = entry_[entry_num_]->begin();
            return *this;
          }
        
        t_ = Term(0);
        return *this;
      }

      t_ = Term(p);
      
      return(*this);
    }

//     const_iterator operator++(int)
//     {
//       const_iterator tmp(*this);

//       char* p = t_.next();
//       if (p - entry_[entry_num_]->get_buf()>=entry_[entry_num_]->size())
//       {
//         for (; entry_num_<entry_size_; ++entry_num_)
//           if (entry_[entry_num_]!=NULL)
//           {
//             t_ = entry_[entry_num_]->begin();
//             return *this;
//           }
//         t_ = Term(0);
//         return *this;
//       }

//       t_ = Term(p);

//       return(tmp);
//     }

    bool operator != (const const_iterator& other)
    {
      return entry_ != other.entry_ || entry_size_ != other.entry_size_ || entry_num_ != other.entry_num_ || t_ != other.t_;
    }
    
  };

  const_iterator begin()
  {
    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!= NULL)
        return const_iterator((const Row**)entry_, entry_size_, i);

    return end();
  }

  const_iterator end()
  {
    return const_iterator((const Row**)entry_, entry_size_, entry_size_);
  }
  
    
  inline TermHashTable(uint32_t entry_size)
  {
    num_ = 0;
    entry_size_ = entry_size;
    
    entry_ = (Row**)malloc(entry_size_*sizeof(Row*));//new Row*[entry_size];
    for (uint32_t i=0;i<entry_size; ++i)
      entry_[i] = NULL;
  }

  inline TermHashTable()
  {
    num_ = 0;
    entry_size_ = 0;
    entry_ = NULL;
  }

  inline ~TermHashTable()
  {
    release();
    free(entry_);
  }
  
  inline void release()
  {
    if (entry_ == NULL)
      return;
    
    for (uint32_t i=0;i<entry_size_; ++i)
      if(entry_[i] != NULL)
      {
        delete entry_[i];
        entry_[i] = NULL;
      }
  }

  uint64_t size()const
  {
    uint64_t s = 0;
    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!=NULL)
        s += entry_[i]->max_size();

    s+= entry_size_*sizeof(Row*);
    
    return s;
  }
  
  uint64_t insert(const char* str, len_t len, uint64_t value)
  {
    register uint32_t convkey = 0;
    register const char* s = str;
    
    for (register uint8_t i = 0; i<len; i++)
      convkey = 37*convkey + *s++;
    convkey %= entry_size_;

    if (entry_[convkey] == NULL)
      entry_[convkey] = new Row(len, convkey);

    
    uint64_t r = entry_[convkey]->add_term(str,len, value);
    if (r == value)
      ++num_;
    
    return r;
  }

  uint64_t find(const char* str, len_t len)
  {
    register uint32_t convkey = 0;
    register const char* s = str;
    
    for (register uint8_t i = 0; i<len; i++)
      convkey = 37*convkey + *s++;
    convkey %= entry_size_;

    if (entry_[convkey] == NULL)
      return -1;

    return entry_[convkey]->find(str, len);

  }
  
  inline void compact()
  {
    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!=NULL)
        entry_[i]->compact();
  }

  inline uint32_t term_num()const
  {
    return num_;
  }
  
  void save(FILE* f)
  {
    uint64_t s = 0;
    uint32_t entry_num = 0;
    
    long int pos1 = ftell(f);
    
    assert(fwrite(&s, sizeof(uint64_t),1, f)==1);
    assert(fwrite(&entry_num, sizeof(uint32_t),1, f)==1);
    assert(fwrite(&entry_size_, sizeof(uint32_t),1, f)==1);
    assert(fwrite(&num_, sizeof(uint32_t),1, f)==1);
    
    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!= NULL)
      {
        ++entry_num;
        s += entry_[i]->save(f);
      }

    long int pos2 = ftell(f);
    
    fseek(f, pos1, SEEK_SET);
    assert(fwrite(&s, sizeof(uint64_t),1, f)==1);
    assert(fwrite(&entry_num, sizeof(uint32_t),1, f)==1);
    //std::cout<<s<<std::endl;

    fseek(f, pos2, SEEK_SET);
  }

  char* load(char* buf, uint32_t buf_size, FILE* f, uint64_t addr=0)
  {
    release();
    uint64_t s = 0;

    fseek(f, addr, SEEK_SET);
    assert(fread(&s, sizeof(uint64_t),1, f)==1);
    if (s > buf_size)
      return NULL;

    uint32_t es = 0;
    //entry number
    assert(fread(&es, sizeof(uint32_t),1, f)==1);
    assert(fread(&es, sizeof(uint32_t),1, f)==1);

    if (entry_==NULL || es != entry_size_)
    {
      entry_size_ = es;
      entry_ = (Row**)malloc(entry_size_*sizeof(Row*));//entry_ = new Row*[entry_size_];
      for (uint32_t i=0;i<entry_size_; ++i)
        entry_[i] = NULL;
    }
    
    assert(fread(&num_, sizeof(uint32_t),1, f)==1);
    assert(fread(buf, s, 1, f)==1);

    uint64_t size = 0;
    while (size<s)
    {
      Row* row = new Row(buf+size);
      entry_[row->index()] = row;
      size += row->size();
    }

    return buf;
  }

  uint32_t load(FILE* f, uint64_t addr=0)
  {
    release();
    
    uint64_t s = 0;
    uint32_t entry_num = 0;

    fseek(f, addr+sizeof(uint64_t), SEEK_SET);
    assert(fread(&entry_num, sizeof(uint32_t),1, f)==1);
    //entry size
    assert(fread(&s, sizeof(uint32_t),1, f)==1);
    
    if (s != entry_size_ )
    {
      entry_size_ = s;
      if (entry_ != NULL)
        delete entry_;
      entry_ = (Row**)malloc(entry_size_*sizeof(Row*));//entry_ = new Row*[entry_size_];
      for (uint32_t i=0;i<entry_size_; ++i)
        entry_[i] = NULL;
    }

    uint32_t size = 0;
    assert(fread(&num_, sizeof(uint32_t),1, f)==1);
    for (uint32_t i=0; i<entry_num; ++i)
    {
      Row* row = new Row(f);
      entry_[row->index()] = row;
      size += row->size();
    }
    
    return size;
  }

friend std::ostream& operator << (std::ostream& os, const SelfT& tb)
  {
    for (uint32_t i=0; i<tb.entry_size_; ++i)
      if (tb.entry_[i]!= NULL)
      {
        os<<(*tb.entry_[i]);
      }
    return os;
  }
  
}
  ;

template <
  uint32_t CACHE_SIZE = 10 //MB
  >
class TermPropertyVector
{
  struct PROPERTY
  {
    char freq_[4];
    char loaded_;
    char child_[8];
    char doc_list_[8];

#define GET_FREQ(buf) (*(uint32_t*)((PROPERTY*)(buf))->freq_)
#define GET_LOADED(buf) (((PROPERTY*)(buf))->loaded_)
#define GET_CHILD(buf) (*(uint64_t*)((PROPERTY*)(buf))->child_)
#define GET_DOC_LIST(buf) (*(uint64_t*)((PROPERTY*)(buf))->doc_list_)
  }
    ;

  char* buf_;
  uint64_t num_;
  uint64_t start_addr_;
  uint64_t p_;
  bool     changed_;
  FILE* f_;

  inline bool is_in_mem(uint64_t addr)
  {
    return addr>=start_addr_ && addr+sizeof(struct PROPERTY) <= start_addr_+p_;
  }

  inline void load_from(uint64_t addr)
  {
    //std::cout<<p_<<" "<<addr<<" "<<start_addr_<<" load...\n";
    if (changed_)
      flush();
    
    fseek(f_, addr, SEEK_SET);
    fread(buf_, CACHE_SIZE*1000000, 1, f_);
    start_addr_ = addr;

    p_ = CACHE_SIZE*1000000;
    if (num_*sizeof(struct PROPERTY)+sizeof(uint64_t) - start_addr_ < p_)
      p_ = num_*sizeof(struct PROPERTY)+sizeof(uint64_t) - start_addr_;
    
    changed_ = false;
  }

  inline void flush()
  {
    std::cout<<"flush...\n";
    fseek(f_, 0, SEEK_SET);
    assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    
    fseek(f_, start_addr_, SEEK_SET);
    assert(fwrite(buf_, p_, 1, f_)==1);
    fflush(f_);
    start_addr_ += p_;
    p_ = 0;
    changed_ = false;
  }
  
public:
  inline TermPropertyVector(const char* nm)
  {
    buf_ = (char*)malloc(CACHE_SIZE*1000000);
    start_addr_ = p_ = num_ = 0;
    changed_ = false;

    f_ = fopen(nm, "r+");
    if (f_ == NULL)
    {
      f_ = fopen(nm, "w+");
      assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    }
    else
    {
      assert(fread(&num_, sizeof(uint64_t), 1, f_)==1);
      load_from(sizeof(uint64_t));
    }
    
    if (f_ == NULL)
    {
      std::cout<<"Can't create file: "<<nm<<std::endl;
      return;
    }

    start_addr_ = sizeof(uint64_t);
  }

  inline ~TermPropertyVector()
  {
    if (changed_)
      flush();
    fclose(f_);
    free(buf_);
  }

  inline uint32_t get_freq(uint64_t addr) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    return GET_FREQ(buf_+addr-start_addr_);
  }

  inline uint8_t get_loaded(uint64_t addr) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    return GET_LOADED(buf_+addr-start_addr_);
  }

  inline uint64_t get_child(uint64_t addr) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    return GET_CHILD(buf_+addr-start_addr_);
  }

  inline uint64_t get_doc_list(uint64_t addr) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    return GET_DOC_LIST(buf_+addr-start_addr_);
  }

  inline void add_freq(uint64_t addr) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    GET_FREQ(buf_+addr-start_addr_)++;
    changed_ = true;
  }

  inline void set_loaded(uint64_t addr, uint8_t loaded) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    GET_LOADED(buf_+addr-start_addr_) = loaded;
    changed_ = true;
  }

  inline void set_child(uint64_t addr, uint64_t child) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    GET_CHILD(buf_+addr-start_addr_) = child;
    changed_ = true;
  }

  inline void set_doc_list(uint64_t addr, uint64_t list) 
  {
    if (!is_in_mem(addr))
      load_from(addr);

    GET_DOC_LIST(buf_+addr-start_addr_) = list;
    changed_ = true;
  }

  inline uint64_t touch_append()const
  {
    return num_*sizeof(struct PROPERTY)+sizeof(uint64_t);
  }
  
  inline uint64_t append(uint32_t freq = 0, uint8_t loaded=0, uint64_t child=-1, uint64_t doc_list=-1)
  {
    if (p_ + sizeof(struct PROPERTY)>=CACHE_SIZE*1000000 && changed_)
      flush();
    
    GET_FREQ(buf_+p_) = freq;
    GET_CHILD(buf_+p_) = child;
    GET_DOC_LIST(buf_+p_) = doc_list;
    GET_LOADED(buf_+p_) = loaded;

    changed_ = true;
    p_ += sizeof(struct PROPERTY);
    num_++;

    return start_addr_ + p_ - sizeof(struct PROPERTY);
  }

  inline uint64_t get_num()const
  {
    return num_;
  }

  inline uint64_t size()const
  {
    return num_*sizeof(struct PROPERTY)+sizeof(uint64_t);
  }
  
}
  ;
NS_IZENELIB_AM_END

#endif

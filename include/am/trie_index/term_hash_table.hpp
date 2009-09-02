#ifndef TERM_HASH_TABLE
#define TERM_HASH_TABLE

#include<types.h>
#include<stdlib.h>
#include<stdio.h>
#include <iostream>
#include <assert.h>

NS_IZENELIB_AM_BEGIN


/**
   len(1)|freq(4)|loaded(1)|child(8)|doc_list(8)|term\0|....
 **/
class Term
{
public:
  typedef uint32_t freq_t;
  typedef uint64_t termid_t;

private:
  char* buf_;

public:
  struct TERM
  {
    char id[sizeof(termid_t)];
    char   freq[sizeof(freq_t)];
    char loaded;
    char child[8];
    char doc_list[8];
  }
    ;
protected:
  
#define GET_ID(buf) (*(termid_t*)((TERM*)(buf))->id)
#define GET_FREQ(buf) (*(freq_t*)((TERM*)(buf))->freq)
#define GET_LOADED(buf) (((TERM*)(buf))->loaded)
#define GET_CHILD(buf) (*(uint64_t*)((TERM*)(buf))->child)
#define GET_DOC_LIST(buf) (*(uint64_t*)((TERM*)(buf))->doc_list)

public:
  inline explicit Term(char* buf)
    :buf_(buf)
  {
  }

  Term(termid_t id,  char* buf, uint64_t child = -1, uint64_t doc_list = -1)
  {
    buf_ = buf;
    GET_ID(buf_) = id;
    GET_FREQ(buf_) = 1;
    GET_LOADED(buf_) = 0;
    GET_CHILD(buf_) = child;
    GET_DOC_LIST(buf_) = doc_list;
    
  }

  bool is_same(const Term& other)
  {
//     std::cout<<get_id()<<" "<<other.get_id()<<std::endl;
//     std::cout<<get_child()<<" "<<other.get_child()<<std::endl;
//     std::cout<<frequency()<<" "<<other.frequency()<<std::endl;
//     std::cout<<get_doc_list()<<" "<<other.get_doc_list()<<std::endl;
//     std::cout<<"ooooooooooooooooo\n";
    
    if (get_id()!=other.get_id()
        ||get_loaded()==other.get_loaded() && get_child()!=other.get_child()
        ||frequency() !=other.frequency())
      return false;
    return true;
  }
  
  static inline uint32_t need_size()
  {
    return sizeof(struct TERM);
  }
  
  static inline uint32_t size()
  {
    return sizeof(struct TERM);
  }
  
  inline freq_t frequency()const
  {
    return GET_FREQ(buf_);
  }

  inline bool get_loaded()const
  {
    return GET_LOADED(buf_);
  }

  inline uint64_t get_child()const
  {
    return GET_CHILD(buf_);
  }

  inline uint64_t get_doc_list()const
  {
    return GET_DOC_LIST(buf_);
  }

  inline termid_t get_id() const
  {
    return GET_ID(buf_);
  }

  inline char* next() const
  {
    return buf_+sizeof(struct TERM);
  }
  
  bool operator == (const Term& term)
  {
    return get_id() == term.get_id();
  }

  bool operator == (const char* str)
  {
    return get_id() == GET_ID(str);
  }
  
  inline void add_freq()
  {
    GET_FREQ(buf_)++;
  }

  inline uint32_t get_freq()const
  {
    return GET_FREQ(buf_);
  }

  inline void set_freq(uint32_t freq)const
  {
    GET_FREQ(buf_) = freq;
  }

  inline void set_loaded(bool b)
  {
    GET_LOADED(buf_) = b;
  }
  
  inline void set_child(uint64_t child)
  {
    GET_CHILD(buf_) = child;
  }

  inline void set_doc_list(uint64_t list)
  {
    GET_DOC_LIST(buf_) = list;
  }

  
  /**
     If it's not equal, return next term, if it is, return current term.
   **/
  inline bool is_equal(uint64_t id, char** t)
  {
    if (get_id()!=id)
    {
      *t = next();
      return false;
    }

    *t = (buf_);
    return true;
  }

  bool operator != (const Term& other)
  {
    return buf_!=other.buf_;
  }

  inline bool is_null()const
  {
    return buf_==NULL;
  }
  
friend std::ostream& operator << (std::ostream& os, const Term& t)
  {
    os<<"<"<<GET_ID(t.buf_)<<" "<<GET_FREQ(t.buf_)<<" "<<(int)GET_LOADED(t.buf_)<<" "<<GET_CHILD(t.buf_)<<" "<<GET_DOC_LIST(t.buf_)<<">";
    
    return os;
  }

  
}  ;


/**
   max_size(4)|size(4)|Term...
   max_size is the size of the entire buffer ;
 **/
class Row
{
  struct ROW_HEAD
  {
    uint32_t max_size;
    uint32_t size;
  }
    ;

  char* buf_;

#define GET_MAX_SIZE(buf) (((ROW_HEAD*)(buf))->max_size)
#define GET_SIZE(buf) (((ROW_HEAD*)(buf))->size)
public:
  
  inline Row(Term::termid_t id)
  {
    buf_ = (char*)malloc(sizeof (struct ROW_HEAD) + sizeof(struct Term::TERM));
    GET_MAX_SIZE(buf_) = sizeof (struct ROW_HEAD)+sizeof(struct Term::TERM);
    GET_SIZE(buf_) = sizeof (struct ROW_HEAD)+sizeof(struct Term::TERM);
    Term(id, buf_+sizeof(struct ROW_HEAD));
  }
  
  inline Row(FILE* f)
  {
    uint32_t size = 0;
    assert(fread(&size, sizeof(uint32_t), 1, f)==1);
    buf_ = (char*)malloc(size);
    GET_MAX_SIZE(buf_) = size;
    
    assert(fread(buf_+sizeof(uint32_t), size-sizeof(uint32_t), 1, f)==1);
  }

  inline Row(const Row& row)
  {
    uint32_t s = row.size();
    buf_ = (char*)malloc(s);
    memcpy(buf_, row.buf_, s);
    GET_MAX_SIZE(buf_) = s;
  }

  bool operator == (const Row& other)const
  {
    if (count()!= other.count())
      return false;

    uint32_t c = count();
    for (uint32_t i=0; i<c; ++i)
      if (!Term((*this)[i]).is_same(Term(other[i])) )
        return false;
    
    return true;
  }
  
  inline ~Row()
  {
    free(buf_);
  }

  inline uint32_t max_size()const
  {
    return GET_MAX_SIZE(buf_);
  }

  inline uint32_t size() const
  {
    return GET_SIZE(buf_);
  }

  inline static uint32_t head_size()
  {
    return sizeof (struct ROW_HEAD);
  }

  inline uint32_t count()const
  {
    return (GET_SIZE(buf_) - sizeof (struct ROW_HEAD))/sizeof(struct Term::TERM);
  }
  
  inline char* append_term(const Term::termid_t id)
  {
    while (max_size()-size()<= Term::need_size())
    {
      buf_ = (char*)realloc(buf_, 2*size());
      GET_MAX_SIZE(buf_) = 2*size();
    }

    char* r = buf_+size();
    Term t(id, r);
    GET_SIZE(buf_) = t.size();//add size

    return r;
  }
  
  char* find(Term::termid_t id)
  {
    char* p = buf_ + sizeof(struct ROW_HEAD);
    uint32_t num = count();
    
    for (uint32_t i=0; i<num; ++i)
    {
      Term t(p);
      if (t.is_equal(id, &p))
        return p;
    }

    return NULL;
  }

  char* add_term(const Term::termid_t id, bool& is_new)
  {
    is_new = false;
    
    char* p = buf_+sizeof(struct ROW_HEAD);
    uint32_t num = count();
    for (uint32_t i=0; i<num; ++i)
    {
      Term t(p);
      if (t.get_id() == id)
      {
        t.add_freq();
        return p;
      }
      p = t.next();
    }
    
    while (max_size()-size()<= Term::need_size())
    {
      buf_ = (char*)realloc(buf_, 2*size());
      GET_MAX_SIZE(buf_) = 2*size();
    }

    p = buf_+size();
    Term t(id, p);
    GET_SIZE(buf_) += t.size();//add size
    
    is_new = true;

    return p;
    
  }
  
  inline const char* get_buf()const
  {
    return buf_;
  }

  inline char*  begin()const
  {
    return (buf_ + sizeof(struct ROW_HEAD));
  }

  inline void compact()
  {
    uint32_t s = size();
    if (max_size()-s<=1)
      return;

    buf_ = (char*)realloc(buf_, s);
    GET_MAX_SIZE(buf_) = s;
  }

  inline uint32_t save(FILE* f)
  {
    uint32_t s = GET_MAX_SIZE(buf_);
    GET_MAX_SIZE(buf_) = size();
    assert(fwrite(buf_, size(), 1, f)==1);
    GET_MAX_SIZE(buf_) = s;
    
    return size();
  }

  inline uint32_t save(char* buf)
  {
    uint32_t s = GET_MAX_SIZE(buf_);
    GET_MAX_SIZE(buf_) = size();
    memcpy(buf, buf_, size());
    GET_MAX_SIZE(buf_) = s;
    
    return size();
  }

  char* operator [] (uint32_t i)const
  {
    assert(i<count());
    return begin()+i*sizeof(struct Term::TERM);
  }
  
friend std::ostream& operator << (std::ostream& os, const Row& row)
  {
    os<<"\nmax_size:"<<row.max_size()<<"|size:"<<row.size()<<"|count:"<<row.count()<<"\n----------\n";

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

public:
  class const_iterator
  {
    const Row**    entry_;
    uint32_t file_pos_;
    const uint32_t entry_size_;
    uint32_t entry_num_;
    Term t_;

  public:
    const_iterator(const Row** entry, uint32_t entry_size, uint32_t entry_num)
      : entry_(entry), file_pos_(sizeof(uint32_t)*3 + sizeof(uint64_t)+sizeof(uint32_t)+Row::head_size()),
        entry_size_ (entry_size), entry_num_(entry_num),
        t_(entry_num< entry_size? Term(entry_[entry_num_]->begin()): Term(0))
    {
    }

    
    const_iterator(const const_iterator& other)
      : entry_(other.entry_), file_pos_(other.file_pos_), entry_size_ (other.entry_size_),
        entry_num_(other.entry_num_), t_(other.t_)
    {
    }

    Term& operator *()
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
            t_ = Term(entry_[entry_num_]->begin());
            file_pos_ += sizeof(Term::TERM) + sizeof(uint32_t) + Row::head_size();
            return *this;
          }
        
        t_ = Term(0);
        return *this;
      }

      t_ = Term(p);
      file_pos_ += sizeof(Term::TERM);
      
      return(*this);
    }

    inline uint32_t file_pos()const
    {
      return file_pos_;
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

  TermHashTable(const SelfT& other)
  {
    num_ = other.term_num();
    entry_size_ = 0;
    entry_ = NULL;
    
    if (other.entry_size_ != entry_size_ )
    {
      entry_size_ = other.entry_size_;
      if (entry_ != NULL)
        delete entry_;
      entry_ = (Row**)malloc(entry_size_*sizeof(Row*));//entry_ = new Row*[entry_size_];
      for (uint32_t i=0;i<entry_size_; ++i)
        entry_[i] = NULL;
    }
    
    for (uint32_t i=0; i<entry_size_; ++i)
    {
      if (other.entry_[i]==NULL)
      {
        entry_[i] = NULL;
        continue;
      }
      
      Row* row = new Row(*other.entry_[i]);
      entry_[i] = row;
    }

  }

  SelfT& operator = (const SelfT& other)
  {
    num_ = other.term_num();
    
    if (other.entry_size_ != entry_size_ )
    {
      entry_size_ = other.entry_size_;
      if (entry_ != NULL)
        delete entry_;
      entry_ = (Row**)malloc(entry_size_*sizeof(Row*));//entry_ = new Row*[entry_size_];
      for (uint32_t i=0;i<entry_size_; ++i)
        entry_[i] = NULL;
    }
    
    for (uint32_t i=0; i<entry_size_; ++i)
    {
      if (other.entry_[i]==NULL)
      {
        entry_[i] = NULL;
        continue;
      }
      
      Row* row = new Row(*other.entry_[i]);
      entry_[i] = row;
    }

    return *this;

  }
  
  inline ~TermHashTable()
  {
    if (entry_ == NULL)
      return;
    
    release();
    free(entry_);
  }

  bool operator == (const SelfT& other)
  {
    if (other.entry_size_ != entry_size_ )
      return false;

    for (uint32_t i=0; i<entry_size_; ++i)
    {
      if (other.entry_[i]!=entry_[i])
        if (other.entry_[i]==NULL || entry_[i]==NULL)
          return false;

      if (other.entry_[i]==NULL)
        continue;
      
      if (!(*other.entry_[i] == *entry_[i]) )
        return false;
    }

    return true;
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
  
  char* insert(Term::termid_t id, bool& is_new)
  {
    is_new = false;
    
    uint32_t convkey = id%entry_size_;

    if (entry_[convkey] == NULL)
    {
      entry_[convkey] = new Row(id);
      is_new = true;
      ++num_;
      return entry_[convkey]->begin();
    }

    char* r = entry_[convkey]->add_term(id, is_new);
    if (is_new)
      ++num_;
    
    return r;
  }

  char* find(Term::termid_t id)
  {
    uint32_t convkey = id%entry_size_;

    if (entry_[convkey] == NULL)
      return NULL;

    return entry_[convkey]->find(id);

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
  
  uint64_t save(FILE* f)
  {
    uint64_t s = sizeof(uint32_t)*3 + sizeof(uint64_t);
    uint32_t entry_num = 0;

    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!= NULL)
      {
        ++entry_num;
        s += entry_[i]->size() + sizeof(uint32_t);
      }

    char* buf = (char*)malloc(s);
    char* p = buf;

    *(uint64_t*)p = s;
    p += sizeof(uint64_t);
    
    *(uint32_t*)p = entry_num;
    p += sizeof(uint32_t);

    *(uint32_t*)p = entry_size_;
    p += sizeof(uint32_t);
    
    *(uint32_t*)p = num_;
    p += sizeof(uint32_t);
        
    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!= NULL)
      {
        *(uint32_t*)p = i;
        p += sizeof(uint32_t);
        p += entry_[i]->save(p);
      }
    

    
    assert(fwrite(buf, s, 1, f)==1);
    free(buf);
    
    return s;
  }
    
//   uint64_t save(FILE* f)
//   {
//     uint64_t s = sizeof(uint32_t)*3 + sizeof(uint64_t);
//     uint32_t entry_num = 0;
    
//     long int pos1 = ftell(f);
    
//     assert(fwrite(&s, sizeof(uint64_t),1, f)==1);
//     assert(fwrite(&entry_num, sizeof(uint32_t),1, f)==1);
//     assert(fwrite(&entry_size_, sizeof(uint32_t),1, f)==1);
//     assert(fwrite(&num_, sizeof(uint32_t),1, f)==1);
    
//     for (uint32_t i=0; i<entry_size_; ++i)
//       if (entry_[i]!= NULL)
//       {
//         ++entry_num;
//         assert(fwrite(&i, sizeof(uint32_t),1, f)==1);
//         s += entry_[i]->save(f);
//       }

//     long int pos2 = ftell(f);
    
//     fseek(f, pos1, SEEK_SET);
//     assert(fwrite(&s, sizeof(uint64_t),1, f)==1);
//     assert(fwrite(&entry_num, sizeof(uint32_t),1, f)==1);
//     //std::cout<<s<<std::endl;

//     fseek(f, pos2, SEEK_SET);
//     //std::cout<<"sssss "<<pos1<<"-"<<ftell(f)<<std::endl;

//     return s;
//   }

  uint64_t touch_save(FILE* f) const
  {
    uint64_t s = sizeof(uint32_t)*3 + sizeof(uint64_t);

    for (uint32_t i=0; i<entry_size_; ++i)
      if (entry_[i]!= NULL)
        s += entry_[i]->size() + sizeof(uint32_t);

    fseek(f, s, SEEK_CUR);
    
    //std::cout<<ftell(f)<<std::endl;
    
    return s;

  }
  
//   char* load(char* buf, uint32_t buf_size, FILE* f, uint64_t addr=0)
//   {
//     release();
//     uint64_t s = 0;

//     fseek(f, addr, SEEK_SET);
//     assert(fread(&s, sizeof(uint64_t),1, f)==1);
//     if (s > buf_size)
//       return NULL;

//     uint32_t es = 0;
//     //entry number
//     assert(fread(&es, sizeof(uint32_t),1, f)==1);
//     assert(fread(&es, sizeof(uint32_t),1, f)==1);

//     if (entry_==NULL || es != entry_size_)
//     {
//       entry_size_ = es;
//       entry_ = (Row**)malloc(entry_size_*sizeof(Row*));//entry_ = new Row*[entry_size_];
//       for (uint32_t i=0;i<entry_size_; ++i)
//         entry_[i] = NULL;
//     }
    
//     assert(fread(&num_, sizeof(uint32_t),1, f)==1);
//     assert(fread(buf, s, 1, f)==1);

//     uint64_t size = 0;
//     while (size<s)
//     {
//       Row* row = new Row(buf+size);
//       entry_[row->index()] = row;
//       size += row->size();
//     }

//     return buf;
//   }

  uint32_t load(FILE* f, uint64_t addr=0)
  {
    //std::cout<<addr<<"-";
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
      uint32_t e = 0;
      assert(fread(&e, sizeof(uint32_t),1, f)==1);
      Row* row = new Row(f);
      entry_[e] = row;
      size += row->size();
    }

    //std::cout<<ftell(f)<<" load...\n";
    return size;
  }

friend std::ostream& operator << (std::ostream& os, const SelfT& tb)
  {
    for (uint32_t i=0; i<tb.entry_size_; ++i)
      if (tb.entry_[i]!= NULL)
      {
        os<<i<<" ########\n";
        os<<(*tb.entry_[i]);
      }
    return os;
  }
  
}
  ;

NS_IZENELIB_AM_END

#endif

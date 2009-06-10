#ifndef DEQUE_STRING_HPP
#define DEQUE_STRING_HPP

#include <hlmalloc.h>
#include <types.h>
#include <iterator>
#include "algo.hpp"
#include <vector>
#include <ostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

//using namespace std;

NS_IZENELIB_AM_BEGIN

template <
  class CHAR_TYPE = char,
  int COPY_ON_WRITE = 1,
  uint32_t BUCKET_BYTES = 64 //BYTES
  >
class deque_string
{
public:
  typedef CHAR_TYPE value_type;
  typedef CHAR_TYPE CharT;
  typedef uint8_t  ReferT;
  typedef deque_string<CHAR_TYPE, COPY_ON_WRITE, BUCKET_BYTES> SelfT;
  typedef std::size_t size_t;
  enum 
    {
      BUCKET_LENGTH = BUCKET_BYTES/sizeof(CharT)
    };
    
  
  static const size_t npos;

  class const_iterator;
  
  class iterator :
    public std::iterator<std::forward_iterator_tag, CharT>
  {

  friend class const_iterator;
  public:
    iterator(SelfT* obj=NULL, const uint64_t* entry_array=NULL, size_t entry_i=-1, const size_t ii=0)
      : entry_array_(entry_array), entry_i_(entry_i), ii_(ii), obj_(obj)
    {
    }
    
    ~iterator() {}

    // The assignment and relational operators are straightforward
    iterator& operator = (const iterator& other)
    {
      entry_array_ = other.entry_array_;
      entry_i_ = other.entry_i_;
      ii_ = other.ii_;
      obj_ = other.obj_;
      
      return(*this);
    }

    bool operator==(const iterator& other)const
    {
      return(ii_ == other.ii_ && entry_i_ == other.entry_i_ && entry_array_==other.entry_array_);
    }

    bool operator != (const iterator& other)const
    {
      return(ii_ != other.ii_ || entry_i_ != other.entry_i_ || entry_array_!=other.entry_array_);
    }

    bool operator < (const iterator& other)const
    {
      return entry_i_ < other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ < other.ii_;
    }
    
    bool operator > (const iterator& other)const
    {
      return entry_i_ > other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ > other.ii_;
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    iterator& operator++()
    {
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return *this;
      }

      ii_++;
            
      return(*this);
    }

    iterator operator++(int)
    {
      iterator tmp(*this);
      
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return tmp;
      }

      ii_++;

      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    iterator& operator--()
    {
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return *this;
      }

      ii_--;

      return(*this);
    }

    iterator operator--(int)
    {
      iterator tmp(*this);
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return tmp;
      }

      ii_--;

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    CharT& operator*()
    {
      if (obj_->is_refered())
      {
        (*obj_).assign_self();
        entry_array_ = obj_->get_entry_array();
      }
      
      return ((CharT*)(entry_array_[entry_i_]))[ii_];
    }

    uint64_t operator - (const iterator& other) const
    {
      if (entry_array_ != other.entry_array_)
        return -1;

      return entry_i_*BUCKET_LENGTH+ii_ - other.entry_i_*BUCKET_LENGTH+other.ii_;
    }

    iterator operator + (const int& gap) 
    {
      iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t += gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }

    iterator operator - (const int& gap) 
    {
      iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t -= gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }
    
  protected:
    const uint64_t* entry_array_;
    size_t entry_i_;
    size_t ii_;
    SelfT* obj_;
  };
  
  class const_iterator :
    public std::iterator<std::forward_iterator_tag, CharT>
  {
    
  public:
    const_iterator(const uint64_t* entry_array=NULL, size_t entry_i=-1, const size_t ii=0)
      : entry_array_(entry_array), entry_i_(entry_i), ii_(ii)
    {
    }

    const_iterator(const iterator& other)
    {
      entry_array_ = other.entry_array_;
      entry_i_ = other.entry_i_;
      ii_ = other.ii_;
    }
    
    
    ~const_iterator() {}

    // The assignment and relational operators are straightforward
    const_iterator& operator = (const const_iterator& other)
    {
      entry_array_ = other.entry_array_;
      entry_i_ = other.entry_i_;
      ii_ = other.ii_;
      
      return(*this);
    }

    bool operator==(const const_iterator& other)const
    {
      return(ii_ == other.ii_ && entry_i_ == other.entry_i_ && entry_array_==other.entry_array_);
    }

    bool operator==(const iterator& other)const
    {
      return(ii_ == other.ii_ && entry_i_ == other.entry_i_ && (uint64_t*)entry_array_==other.entry_array_);
    }

    bool operator!=(const const_iterator& other)const
    {
      return(ii_ != other.ii_ || entry_i_ != other.entry_i_ || entry_array_!=other.entry_array_);
    }

    bool operator < (const const_iterator& other)const
    {
      return entry_i_ < other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ < other.ii_;
    }
    
    bool operator > (const const_iterator& other)const
    {
      return entry_i_ > other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ > other.ii_;
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator++()
    {
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return *this;
      }

      ii_++;
            
      return(*this);
    }

    const_iterator operator++(int)
    {
      const_iterator tmp(*this);
      
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return tmp;
      }

      ii_++;

      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator--()
    {
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return *this;
      }

      ii_--;
      
      return(*this);
    }

    const_iterator operator--(int)
    {
      const_iterator tmp(*this);
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return tmp;
      }

      ii_--;

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const CharT& operator*()const
    {
      return ((CharT*)(entry_array_[entry_i_]))[ii_];
    }

    uint64_t operator - (const const_iterator& other) const
    {
      if (entry_array_ != other.entry_array_)
        return -1;

      return entry_i_*BUCKET_LENGTH+ii_ - other.entry_i_*BUCKET_LENGTH+other.ii_;
    }

    const_iterator operator + (const int& gap) const
    {
      const_iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t += gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }

    const_iterator operator - (const int& gap)
    {
      const_iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t -= gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }
    
  protected:
    const uint64_t* entry_array_;
    size_t entry_i_;
    size_t ii_;
  };

  
  class const_reverse_iterator;
  
  class reverse_iterator :
    public std::iterator<std::forward_iterator_tag, CharT>
  {

  friend class const_reverse_iterator;
  public:
    reverse_iterator(SelfT* obj=NULL, const uint64_t* entry_array=NULL, size_t entry_i=-1, const size_t ii=0)
      : entry_array_(entry_array), entry_i_(entry_i), ii_(ii), obj_(obj)
    {
    }
    
    ~reverse_iterator() {}

    // The assignment and relational operators are straightforward
    reverse_iterator& operator = (const reverse_iterator& other)
    {
      entry_array_ = other.entry_array_;
      entry_i_ = other.entry_i_;
      ii_ = other.ii_;
      obj_ = other.obj_;
      
      return(*this);
    }

    bool operator==(const reverse_iterator& other)const
    {
      return(ii_ == other.ii_ && entry_i_ == other.entry_i_ && entry_array_==other.entry_array_);
    }

    bool operator!=(const reverse_iterator& other)const
    {
      return(ii_ != other.ii_ || entry_i_ != other.entry_i_ || entry_array_!=other.entry_array_);
    }

    bool operator < (const reverse_iterator& other)const
    {
      return entry_i_ > other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ > other.ii_;
    }
    
    bool operator > (const reverse_iterator& other)const
    {
      return entry_i_ < other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ < other.ii_;
      
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    reverse_iterator& operator--()
    {
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return *this;
      }

      ii_++;
      
      return(*this);
    }

    reverse_iterator operator--(int)
    {
      reverse_iterator tmp(*this);
      
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return tmp;
      }

      ii_++;

      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    reverse_iterator& operator++()
    {
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return *this;
      }

      ii_--;

      return(*this);
    }

    reverse_iterator operator++(int)
    {
      reverse_iterator tmp(*this);
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return tmp;
      }

      ii_--;

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    CharT& operator*()
    {
      if (obj_->is_refered())
      {
        (*obj_).assign_self();
        entry_array_ = obj_->get_entry_array();
      }
      
      return ((CharT*)(entry_array_[entry_i_]))[ii_];
    }

    uint64_t operator - (const reverse_iterator& other) const
    {
      if (entry_array_ != other.entry_array_)
        return -1;

      return other.entry_i_*BUCKET_LENGTH+other.ii_ - entry_i_*BUCKET_LENGTH+ii_;
    }

    reverse_iterator operator - (const int& gap)
    {
      reverse_iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t += gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }

    reverse_iterator operator + (const int& gap)
    {
      reverse_iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t -= gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }
    
  protected:
    const uint64_t* entry_array_;
    size_t entry_i_;
    size_t ii_;
    SelfT* obj_;
  };
  
  class const_reverse_iterator :
    public std::iterator<std::forward_iterator_tag, CharT>
  {
    
  public:
    const_reverse_iterator(const uint64_t* entry_array=NULL, size_t entry_i=-1, const size_t ii=0)
      : entry_array_(entry_array), entry_i_(entry_i), ii_(ii)
    {
    }
    const_reverse_iterator(const reverse_iterator& other)
    {
      entry_array_ = other.entry_array_;
      entry_i_ = other.entry_i_;
      ii_ = other.ii_;
    }
    
    
    ~const_reverse_iterator() {}

    // The assignment and relational operators are straightforward
    const_reverse_iterator& operator = (const const_reverse_iterator& other)
    {
      entry_array_ = other.entry_array_;
      entry_i_ = other.entry_i_;
      ii_ = other.ii_;
      
      return(*this);
    }

    bool operator==(const const_reverse_iterator& other)const
    {
      return(ii_ == other.ii_ && entry_i_ == other.entry_i_ && entry_array_==other.entry_array_);
    }
    
    bool operator==(const reverse_iterator& other)const
    {
      return(ii_ == other.ii_ && entry_i_ == other.entry_i_ && entry_array_==other.entry_array_);
    }

    bool operator!=(const const_reverse_iterator& other)const
    {
      return(ii_ != other.ii_ || entry_i_ != other.entry_i_ || entry_array_!=other.entry_array_);
    }

    bool operator > (const const_reverse_iterator& other)const
    {
      return entry_i_ < other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ < other.ii_;
    }
    
    bool operator < (const const_reverse_iterator& other)const
    {
      return entry_i_ > other.entry_i_ ||
        entry_i_ == other.entry_i_ && ii_ > other.ii_;
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_reverse_iterator& operator--()
    {
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return *this;
      }

      ii_++;
      
      return(*this);
    }

    const_reverse_iterator operator--(int)
    {
      const_reverse_iterator tmp(*this);
      
      if (ii_== BUCKET_LENGTH-1)
      {
        ii_ = 0;
        entry_i_++;
        return tmp;
      }

      ii_++;

      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_reverse_iterator& operator++()
    {
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return *this;
      }

      ii_--;

      return(*this);
    }

    const_reverse_iterator operator++(int)
    {
      const_reverse_iterator tmp(*this);
      if (ii_ == 0)
      {
        entry_i_--;
        ii_= BUCKET_LENGTH -1;
        return tmp;
      }

      ii_--;

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const CharT& operator*()const
    {
      return ((CharT*)(entry_array_[entry_i_]))[ii_];
    }

    uint64_t operator - (const const_reverse_iterator& other) const
    {
      if (entry_array_ != other.entry_array_)
        return -1;

      return other.entry_i_*BUCKET_LENGTH+other.ii_ - entry_i_*BUCKET_LENGTH+ii_;
    }

    const_reverse_iterator operator - (const int& gap) 
    {
      const_reverse_iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t += gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }

    const_reverse_iterator operator + (const int& gap) 
    {
      const_reverse_iterator tmp(*this);
      size_t t = entry_i_*BUCKET_LENGTH+ii_;
      t -= gap;
      tmp.entry_i_ = t/BUCKET_LENGTH;
      tmp.ii_ = t%BUCKET_LENGTH;
      return tmp;
    }
    
  protected:
    const uint64_t* entry_array_;
    size_t entry_i_;
    size_t ii_;
  };

  
protected:
  char* p_;
  size_t start_i_;//globle start position in deque.
  size_t length_;
  size_t max_size_;
  size_t entry_size_;
  CharT** en_array_;

  bool refer()
  {
    if (!COPY_ON_WRITE)
      return false;
    
    //lock    
    if (p_!=NULL)
      (*(ReferT*)p_)++;

    return true;
  }
  
  void derefer()
  { 
    if (!COPY_ON_WRITE)
    {
      if (p_!=NULL)
      {
        hlfree(p_);
        p_ = NULL;
      }
      
      return ;
    }

    //lock
    if (p_!=NULL)
      if (*(ReferT*)p_ > 0)
      (*(ReferT*)p_)--;

    if (p_!=NULL)
      if (*(ReferT*)p_== 0)
      {
        for (size_t i=0; i<entry_size_; i++)
          if (get_entry(i)!=NULL)
          {
            hlfree(get_entry(i));
            set_entry(i, NULL);
          }
        
        hlfree(p_);
        p_ = NULL;
      }
    
  }

  inline void clear_reference()
  {     
    if (!COPY_ON_WRITE)
      return ;

    //lock
    if (p_!=NULL)
      (*(ReferT*)p_) = 1;
  }

  bool is_refered() const
  {
    if (!COPY_ON_WRITE)
      return false;
    
    //lock
    if (p_ == NULL)
      return false;
    
    if (*(ReferT*)p_ > 1)
      return true;

    return false;
  }
   
  void assign_self()
  {
    if (!is_refered())
      return;

    char* p;
    size_t es = init_entry(length_, &p);
    size_t st = BUCKET_LENGTH;
    size_t ei = 0;

    const_iterator it = begin();
    for (size_t i=0; it!=end() && i<length_; i++, it++)
    {
      if ((st+i)%BUCKET_LENGTH==0)
        ei++;
      
      ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
        = *it;
      
    }
        
    derefer();
    p_ = p;
    start_i_ = st;
    entry_size_ = es;
    max_size_ = init_max_size();
    
    clear_reference(); 
  }

  inline size_t getLen(const CharT* s) const
  {
    CharT e = '\0';
    size_t i = 0;
    
    while (s[i]!=e)
    {
      i++;
    }

    return i;
  }

  inline size_t start_entry_index()const
  {
    return (start_i_)/BUCKET_LENGTH;
  }

  inline size_t end_entry_index() const
  {
    return (start_i_ + length_ - (length_==0? 0 : 1))/BUCKET_LENGTH;
  }

  inline size_t char_index_in_bucket(size_t i)const
  {
    if (i==(size_t)-1)
      return 0;
    
    return (start_i_ + i)%BUCKET_LENGTH;
  }
  
  inline size_t get_entry_index(size_t char_index)const
  {
    return (start_i_+char_index)/BUCKET_LENGTH;
  }
  
  inline void set_char(size_t index, CharT c)
  {
    size_t i = index + start_i_;
    (get_entry(i/BUCKET_LENGTH))[i%BUCKET_LENGTH] = c;
  }

  inline CharT& get_char(size_t index)
  {
    size_t i = index + start_i_;
    return ((CharT*)(*(uint64_t*)(p_ + sizeof(ReferT) + i/BUCKET_LENGTH*sizeof(CharT*))))[i%BUCKET_LENGTH];
    //return ((get_entry(i/BUCKET_LENGTH))[i%BUCKET_LENGTH]);
  }

  inline const CharT& get_const_char(size_t index)const
  {
    size_t i = index + start_i_;
    return ((CharT*)(*(uint64_t*)(p_ + sizeof(ReferT) + i/BUCKET_LENGTH*sizeof(CharT*))))[i%BUCKET_LENGTH];
    //return (get_const_entry(i/BUCKET_LENGTH)[i%BUCKET_LENGTH]);
  }

  inline CharT* get_entry(size_t i)
  {
    return (CharT*)(*(uint64_t*)(p_ + sizeof(ReferT) + i*sizeof(CharT*)));
  }

  inline const CharT* get_const_entry(size_t i) const
  {
    return (CharT*)(*(uint64_t*)(p_ + sizeof(ReferT) + i*sizeof(CharT*)));
  }

  inline uint64_t* get_entry_array()
  {
    return (uint64_t*)(p_ + sizeof(ReferT));
  }

  inline const uint64_t* get_const_entry_array() const
  {
    return (uint64_t*)(p_ + sizeof(ReferT));
  }

  inline size_t need_bucket_num(size_t k) const
  {
    return k/BUCKET_LENGTH + (k%BUCKET_LENGTH==0? 0: 1);
  }
  
  inline size_t need_entry_bytes(size_t size)const
  {
    return sizeof(ReferT) + size*sizeof(CharT*);
  }

  inline size_t need_entry_size(size_t length)const
  {
    size_t es = (length + start_i_);
    return need_bucket_num(es);
  }

  inline void set_entry(size_t i, CharT* p)
  {
    *(uint64_t*)(p_ + sizeof(ReferT) + i*sizeof(CharT*)) = (uint64_t)p;
  }

  inline char* malloc_char(uint64_t bytes)
  {
    return (char*)hlmalloc(bytes); 
  }

  inline CharT* malloc_CharT(uint64_t bytes = BUCKET_BYTES)
  {
    return (CharT*)hlmalloc(bytes); 
  }

  size_t init_entry(size_t length, char** pp)
  {
    if (length==0 || length==(size_t)-1)
      length = 1;
    
    size_t es = need_bucket_num(length)+2;
    
    *pp = malloc_char(need_entry_bytes(es));
    *(uint64_t*)(*pp + sizeof(ReferT) + 0*sizeof(CharT*)) = 0;
    
    for (size_t i=1; i<es-1; i++)
      *(uint64_t*)(*pp + sizeof(ReferT) + i*sizeof(CharT*)) = (uint64_t)malloc_CharT();
      
    *(uint64_t*)(*pp + sizeof(ReferT) + (es-1)*sizeof(CharT*)) = 0;

    en_array_ = (CharT**)(*pp+sizeof(ReferT));

    return es;
  }

  size_t init_max_size() const
  {
    return (entry_size_-2)*BUCKET_LENGTH;
  }
  

  void extend_entry(size_t t, bool back=true)
  {
    p_ = (char*)hlrealloc(p_, need_entry_bytes(entry_size_+t));
    
    if (back)
    {
      for (size_t i=0; i<t; i++)
        set_entry(entry_size_+i, NULL);
      entry_size_ += t;
    }
    else
    {
      uint64_t* entry = get_entry_array();
      size_t e = end_entry_index();
      for (size_t i=e; i!=(size_t)-1; i--)
        entry[i+t] = entry[i];
      
      start_i_ += t*BUCKET_LENGTH;
      
      for (size_t i=0; i<t; i++)
        entry[i]= 0;
      entry_size_ += t;
    }
    
  }

  void add_buckets(size_t t, bool back=true)
  {
    //std::cout<<t<<std::endl;
    
    if (t ==0)
      return;

    //t = (size_t)(3*t);
    
    if (back)
    {
      if (end_entry_index()+t >= entry_size_)
        extend_entry(t);
    
      for (size_t i=0; i<t; i++)
      {  
        if (get_entry(end_entry_index()+i+1)==NULL)
        {
          set_entry(end_entry_index()+i+1, malloc_CharT());
          max_size_ += BUCKET_LENGTH;
        }
      }
    }
    else
    {
      if (start_entry_index()<t)
        extend_entry(t-start_entry_index(),false);

      size_t s = start_entry_index()-1;
      for (size_t i=0; i<t; i++)
      {  
        if (get_entry(s-i)==NULL)
        {
          set_entry(s-i, malloc_CharT());
          max_size_ += BUCKET_LENGTH;
        }
      }
    }
  }
  
public:
  explicit deque_string()
  {
    entry_size_ = init_entry(1, &p_);
    start_i_ = BUCKET_LENGTH;
    length_ = 0;
    max_size_ = init_max_size();
    clear_reference();
  }

  deque_string ( const SelfT& str )
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(str);
  }
  
  deque_string ( const SelfT& str, size_t pos, size_t n = npos )
  {
        
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(str, pos, n);
  }
  
  deque_string ( const CharT * s, size_t n )
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(s, n);
  }
  
  deque_string ( const CharT * s )
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(s);
  }
  
  deque_string ( size_t n, CharT c )
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(n, c);
  }

  deque_string (const std::vector<CharT>& v)
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(v);
  }

  deque_string (const std::string& str)
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(str);
  }
  

  template<class InputIterator> deque_string (InputIterator begin, InputIterator end)
  {    
    p_ = NULL;
    start_i_ = -1;
    length_ = 0;
    max_size_ = 0;
    entry_size_ = 0;
    assign(begin(), end);
  }

  virtual ~deque_string()
  {
    derefer();
  }

  ReferT getReferCnt()
  {
    if (p_ != NULL)
      return *(ReferT*)p_;
    return 0;
  }
  
  SelfT& operator= ( const SelfT& str )
  {
    return this->assign(str);
  }

  SelfT& operator= ( const CharT* s )
  {
    return this->assign(s);
  }

  SelfT& operator= ( CharT c )
  {
    this->assign(1, c);
    return *this;
  }

  //******************Iterators********************
public:

  const_iterator begin() const
  {
    return const_iterator(get_const_entry_array(), start_entry_index(),
                          start_i_%BUCKET_LENGTH);
  }

  iterator begin()
  {
    return iterator(this, get_entry_array(), start_entry_index(),
                          start_i_%BUCKET_LENGTH);
  }

  const_iterator end(int i=0) const
  {
    if (length_==0)
      return begin();
    
    return const_iterator(get_const_entry_array(), end_entry_index(),
                          char_index_in_bucket(length_-1))+1;
  }
  
  iterator end()
  {
    if (length_==0)
      return begin();
    
    return iterator(this, get_entry_array(), end_entry_index(),
                    char_index_in_bucket(length_-1))+1;
  }

  reverse_iterator rbegin()
  {
    return reverse_iterator(this, get_entry_array(), end_entry_index(),
                            char_index_in_bucket(length_-1));
  }

  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(get_const_entry_array(), end_entry_index(),
                          char_index_in_bucket(length_-1));
  }

  reverse_iterator rend()
  {
    if (length_==0)
      return rbegin();
    
    return reverse_iterator(this, get_entry_array(), start_entry_index(),
                          start_i_%BUCKET_LENGTH)+1;
  }

  const_reverse_iterator rend() const
  {
    if (length_==0)
      return rbegin();
    
    return const_reverse_iterator(get_const_entry_array(), start_entry_index(),
                          start_i_%BUCKET_LENGTH)+1;
  }


  //******************Capacity********************
public:

  size_t size() const
  {
    return length_ * sizeof(CharT);
  }

  size_t length() const
  {
    return length_;
  }

  size_t max_size ( ) const
  {
    return (max_size_);
  }

  void resize (size_t n, CharT c)
  {
    assert (max_size_>=length_);
    
    if (n == length_ && n == max_size_)
      return;

    if (is_refered())
    {
      char* p;
      size_t es = init_entry(n, &p);
      size_t st = BUCKET_LENGTH;
      
      const_iterator it = begin();
      size_t i=0;
      size_t ei = 0;
      for (;it!= end() && i<n; i++, it++)
      {
        if ((st+i)%BUCKET_LENGTH==0)
          ei++;
      
        ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
          = *it;
      }

      for (;i<n; i++)
        ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
          = c;
            
      derefer();
      start_i_ = st;
      entry_size_ = es;
      max_size_ = init_max_size();
      length_ = n;
      p_ = p;
      clear_reference();
      return;
    }

    if (n > max_size_-start_i_%BUCKET_LENGTH)
    {
      size_t t = n - max_size_-start_i_%BUCKET_LENGTH;
      add_buckets(need_bucket_num(t));
    }
    else if (n < max_size_-start_i_%BUCKET_LENGTH)
    {
      size_t t = get_entry_index(n);
      for (size_t i=t+1; i<entry_size_; i++)
      {
        if (get_entry(i)!=NULL)
        {
          hlfree(get_entry(i));
          set_entry(i, NULL);
          max_size_ -= BUCKET_LENGTH;
        }
      }
    }

    iterator it = end();
    size_t i=length_;
    for (;i<n; i++, it++)
      *it = c;
    length_ = n;
  }
  

  void resize ( size_t n )
  {
    assert (max_size_>=length_);
    
    if (n == max_size_)
    {
      length_ = n;
      return;
    }

    if (is_refered())
    {
      char* p;
      size_t es = init_entry(n, &p);
      size_t st = BUCKET_LENGTH;
      
      const_iterator it = begin();
      size_t i=0;
      size_t ei = 0;
      for (;it!= end() && i<n; i++, it++)
      {
        if ((st+i)%BUCKET_LENGTH==0)
          ei++;
      
        ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
          = *it;
      }
            
      derefer();
      start_i_ = st;
      entry_size_ = es;
      max_size_ = init_max_size();
      length_ = n;
      p_ = p;
      clear_reference();
      return;
    }

    if (n > max_size_-start_i_%BUCKET_LENGTH)
    {
      size_t t = n - max_size_-start_i_%BUCKET_LENGTH;
      add_buckets(need_bucket_num(t));
    }
    else if (n < max_size_-start_i_%BUCKET_LENGTH)
    {
      size_t t = get_entry_index(n-1);
      for (size_t i=t+1; i<entry_size_; i++)
      {
        if (get_entry(i)!=NULL)
        {
          hlfree(get_entry(i));
          set_entry(i, NULL);
          max_size_ -= BUCKET_LENGTH;
        }
      }
    }
    

    length_ = n;
    
  }

  size_t capacity ( ) const
  {
    return max_size_ * sizeof(CharT);
  }

  void reserve ( size_t res_arg=0 )
  {
    if (res_arg<= capacity())
      return;

    size_t len = length_;
    resize(res_arg/sizeof(CharT));
    length_ = len;
  }

  void clear()
  {
    derefer();
    length_ = max_size_ = 0;
    start_i_ = -1;
  }

  bool empty () const
  {
    return (length_==0);
  }

  //******************Element access********************
public:
  const CharT& operator[] ( size_t pos ) const
  {
    assert(pos < length_);
    return get_const_char(pos);
  }
  
  CharT& operator[] ( size_t pos )
  {
    assert(pos<length_);
    assign_self();
    return get_char(pos);
    
  }

  const CharT& at ( size_t pos ) const
  {
    assert(pos<length_);
    return get_char(pos);
  }
  
  CharT& at ( size_t pos )
  {
    assert(pos<length_);
    assign_self();
    return get_char(pos);
  }

  //******************Modifiers********************
public:
  SelfT& operator+= ( const SelfT& str )
  {
    return append(str);
  }
  
  SelfT& operator+= ( const CharT* s )
  {
    size_t len = getLen(s);
    return append(s, len);
  }
  
  SelfT& operator+= ( CharT c )
  {
    return append(1, c);
  }
  
  SelfT& append ( const SelfT& str )
  {
    if (!str.length())
      return *this;

    assign_self();

    if (p_==NULL)
      return assign(str);
    
    if (char_index_in_bucket(length_-1)+1+str.length()>BUCKET_LENGTH)
      add_buckets((char_index_in_bucket(length_-1)+ 1 + str.length())/BUCKET_LENGTH);

    const_iterator j=str.begin();
    size_t g = 0;
    for (iterator i=end();g<str.length();g++,i++,j++)
      *i = *j;

    length_ += str.length();
    clear_reference();
      
    return *this;
  }
  
  SelfT& append ( const SelfT& str, size_t pos, size_t n=npos )
  {
    return append(str.substr(pos, n));
  }
  
  SelfT& append ( const CharT* s, size_t n )
  {
    if (n == 0)
      return *this;
    
    assign_self();

    if (p_==NULL)
    {
      return assign(s, n);
    }
    
    if (char_index_in_bucket(length_-1)+1+n>BUCKET_LENGTH)
      add_buckets((char_index_in_bucket(length_-1)+1+n)/BUCKET_LENGTH);

    
    size_t j=0;
    for (iterator i=end();j<n;i++,j++)
      *i = s[j];

    length_ += n;
    clear_reference();
    
    return *this;
    
  }

  SelfT& append ( const CharT* s )
  {
    size_t len = getLen(s);
    return append(s, len);
  }

  SelfT& append ( size_t n, CharT c )
  {
    if (n==0)
      return *this;
    
    assign_self();
    if (p_==NULL)
    {
      assign(n, c);
      return *this;
    }
    
    if (char_index_in_bucket(length_-1)+1+n>BUCKET_LENGTH)
      add_buckets((char_index_in_bucket(length_-1)+1+n)/BUCKET_LENGTH);

    size_t j=0;
    for (iterator i=end();j<n;i++,j++)
      *i = c;

    length_ += n;
    clear_reference();
    
    return *this;
  }

  template <class InputIterator>
  SelfT& append ( InputIterator first, InputIterator last )
  {
    for (InputIterator i=first(); i!=last; i++)
      appand(1, *i);

    return *this;
  }

  void push_back ( CharT c )
  {
    append(1, c);
  }

  SelfT& push_front (const SelfT& str)
  {
    // SelfT s(str);
//     s += *this;
//     assign(s);
//     return *this;
    
    if (!str.length())
      return *this;

    assign_self();

    if (p_==NULL)
      return assign(str);
    
    if (BUCKET_LENGTH - char_index_in_bucket(0)+1+str.length()>BUCKET_LENGTH)
      add_buckets((BUCKET_LENGTH - char_index_in_bucket(0)+ 1 + str.length())/BUCKET_LENGTH, false);

    const_iterator j=str.end()-1;
    size_t g = str.length();
    for (iterator i=begin()-1;g>0; start_i_--,g--, i--,j--)
      *i = *j;

    length_ += str.length();
    clear_reference();

    return *this;    
  }

  SelfT& push_front (CharT c)
  {
    if (length_ == 0)
      return assign(1, c);
    
    if (start_i_%BUCKET_LENGTH != 0)
    {
      start_i_--;
      set_char(start_i_, c);
      length_++;
      return *this;
    }

    if (get_entry_index(start_i_) == 0)
    {
      p_ = (char*)hlrealloc(p_, need_entry_bytes(++entry_size_));
      //shift entry
      uint64_t* en = get_entry_array();
      for (size_t i=entry_size_-1; i>0; i--)
        en[i] = en[i-1];
      en[0] = (uint64_t)malloc_CharT();
      start_i_ += BUCKET_LENGTH;
      max_size_ += BUCKET_LENGTH;
    }
    
    start_i_--;
    
    if (get_entry(get_entry_index(0))==NULL)
    {
      uint64_t* en = get_entry_array();
      en[0] = (uint64_t)malloc_CharT();
      max_size_ += BUCKET_LENGTH;
    }
    
    set_char(start_i_, c);
    
    length_++;
    return *this;
  }

  SelfT& assign ( const SelfT& str )
  {
    if (p_!=NULL && str.begin() == begin()
        && length_ == str.length()
        && compare(str)==0)
      return *this;
    
    derefer();
      
    if (COPY_ON_WRITE)
    {
      p_ = str.p_;  
      start_i_ = str.start_i_;
      length_ = str.length_;
      max_size_ = str.max_size_;
      entry_size_ = str.entry_size_;
      
      refer();
      
      assert(length_<=max_size_);
    }
    else
    {
      entry_size_ = init_entry(str.length(), &p_);
      start_i_ = BUCKET_LENGTH;

      const_iterator j=str.begin();
      size_t g = 0;
      for (iterator i=begin();g<str.length(); g++, j++, i++)
        *i = *j;
        
      max_size_ = init_max_size();
      length_ = str.length_;
            
      clear_reference();
    }

    return *this;
  }
  
  SelfT& assign ( const SelfT& str, size_t pos, size_t n )
  {
    if (n != npos)
      assert(str.length_ >= pos+n);

    derefer();
    
    if (n == npos)
        length_ = str.length_ - pos;
    else
        length_ = n;

    if (COPY_ON_WRITE)
    {
      p_ = str.p_;
      refer();
      
      start_i_ = str.start_i_ + pos;
      max_size_ = str.max_size_;
      entry_size_ = str.entry_size_;
    }
    else
    {
      entry_size_ = init_entry(length_, &p_);
      start_i_ = BUCKET_LENGTH;

      const_iterator j=str.begin()+pos;
      size_t g = pos;
      for (iterator i=begin();g<length_+pos;g++, j++, i++)
        *i = *j;
        
      max_size_ = init_max_size();
            
      clear_reference();
    }

    return *this;
  }
  
  SelfT& assign ( const CharT* s, size_t n )
  {
    derefer();
    
    entry_size_ = init_entry(n, &p_);
    start_i_ = BUCKET_LENGTH;
    length_ = n;
    
    for (iterator i=begin();n!=0; s++, n--, i++)
      *i = *s;
        
    max_size_ = init_max_size();
            
    clear_reference();
      
    return *this;
  }

  SelfT& assign ( const CharT* s )
  {
    assert (s !=NULL);

    return assign(s, getLen(s));
  }

  SelfT& assign ( size_t n, CharT c )
  {
    derefer();
    
    entry_size_ = init_entry(n, &p_);
    start_i_ = BUCKET_LENGTH;
    length_ = n;
    
    for (iterator i=begin();n!=0;n--, i++)
      *i = c;

    max_size_ = init_max_size();
            
    clear_reference();
    
    return *this;
  }

  SelfT& assign (const std::vector<CharT>& v)
  {
    derefer();
        
    entry_size_ = init_entry(v.size(), &p_);
    start_i_ = BUCKET_LENGTH;
    length_ = v.size();

    typename std::vector<CharT>::const_iterator j = v.begin();
    for (iterator i=begin(); j!=v.end();j++, i++)
      *i = *j;
        
    max_size_ = init_max_size();
            
    clear_reference();

    return *this;
  }

  SelfT& assign (const std::string& str)
  {
    derefer();
    
    entry_size_ = init_entry(str.length(), &p_);
    start_i_ = BUCKET_LENGTH;
    length_ = str.length();

    typename std::string::const_iterator j = str.begin();
    for (iterator i=begin(); j!=str.end();j++, i++)
      *i = *j;
        
    max_size_ = init_max_size();
            
    clear_reference();

    return *this;
  }
  
  template <class InputIterator>
  SelfT& assign ( InputIterator first, InputIterator last )
  {
    derefer();
    
    InputIterator i=first;
    if (i!=last)
    {
      assign(1, *i);
      i++;
    }
    
    for (; i!=last; i++)
      append(1, *i);

    return *this;
  }

  SelfT& insert ( size_t pos1, const SelfT& str )
  {
    if (str.length() == 0)
      return *this;

    assert(pos1<length_);

    if (pos1==length_-1)
    {
      append(str);
      return *this;
    }

    return replace(pos1+1,0 , str);
  }
  
  SelfT& insert ( size_t pos1, const SelfT& str, size_t pos2, size_t n )
  {
    return insert(pos1, str.substr(pos2, n));
  }
  
  SelfT& insert ( size_t pos1, const CharT* s, size_t n)
  {
    SelfT ss(s, n);
    //ss.attach(s, n);
    return insert(pos1, ss);
  }
  
  SelfT& insert ( size_t pos1, const CharT* s )
  {
    SelfT ss(s);
    //ss.attach(s);
    return insert(pos1, ss);
  }
  
  SelfT& insert ( size_t pos1, size_t n, CharT c )
  {
    SelfT ss(n, c);
    return insert(pos1, ss);
  }
  
  iterator insert ( iterator p, CharT c )
  {
    uint64_t i = p - begin();
    assign_self();
    insert(i, 1, c);
    return iterator(begin()+i+1);
  }
  
  void insert ( iterator p, size_t n, char c )
  {
    uint64_t i = p - begin();
    assign_self();
    insert(i, n, c);
    return iterator(begin()+i+1);
  }
  
  template<class InputIterator>
  void insert( iterator p, InputIterator first, InputIterator last )
  {
    uint64_t i = p - begin();
    assign_self();
    
    SelfT s;
    s.assign(first, last);
    
    insert(i, s);
  }

  SelfT& erase ( size_t pos = 0, size_t n = npos )
  {
    assign_self();
    if (p_==NULL)
        return *this;

    if (pos ==0)
    {
      length_ -= (n==npos? length_: n);
      start_i_ += (n==npos? length_: n);
      
      return *this;
    }

    assert(pos<length_);
    
    if (n >= length_ - pos)
    {
      length_ = pos;
      return *this;
    }

      
    for (iterator i=begin()+pos, j=begin()+pos+n; j<end(); i++, j++)
      *i = *j;

    length_ -= n;
    clear_reference();

    return *this;
  }
  
  iterator erase ( iterator position )
  {    
    uint64_t i = position - begin();
    erase(i, 1);
    return iterator(begin()+i);
  }
  
  iterator erase ( iterator first, iterator last )
  {
    uint64_t i = first - begin();
    uint64_t j = last - begin();
    erase(i, j-i);
    return iterator(begin()+i);
  }
  
protected:
  SelfT& replace_ ( size_t pos1, size_t n1, SelfT str )
  {        
    int r = BUCKET_LENGTH- char_index_in_bucket(pos1);
    if (r < (int)str.length())
      r += (str.length()-r)%BUCKET_LENGTH;
    else r = str.length();
    
    size_t n = BUCKET_LENGTH- char_index_in_bucket(pos1);
    if (n < n1)
      n += (n1-n)%BUCKET_LENGTH;
    else n = n1;

    r -= n;

    if (r>0)
    {
      //std::cout<<r<<" "<<char_index_in_bucket(length_-1)<<std::endl;
      
      size_t y = (BUCKET_LENGTH - char_index_in_bucket(length_-1)-1);
      if ((size_t)r > y)
        add_buckets(need_bucket_num(r-y));

      // move
      size_t pos = length_-1;
      for (iterator i=end()-1, j=end()-1+r; pos>=pos1+n1;pos--,i--,j--)
      {
        *j = *i;
      }
      
    }
    else
    {
      //move
      for (iterator i=begin()+pos1+n1, j=begin()+(pos1+n1+r); i<end();j++, i++)
        *j = *i;
    }
    
    length_ += r;

    // add buckets
    if (str.length()>n1 && str.length() + char_index_in_bucket(pos1) - n1 >= 2*BUCKET_LENGTH)
    {      
      size_t k = (str.length() + char_index_in_bucket(pos1)-n1)/BUCKET_LENGTH-1;
      size_t m = get_entry_index(pos1);

      if (end_entry_index()+k>=entry_size_)
      {
        extend_entry(k);
      }
      
      uint64_t* entry = get_entry_array();
      for (size_t i=end_entry_index()+k; i-k>m; i--)
        entry[i] = entry[i-k];

      for (size_t i=0; i<k; i++)
        entry[m+1+i] = (uint64_t)malloc_CharT();
      
      length_ += k*BUCKET_LENGTH;
      max_size_ += k;
    }

    //delete bucket
    if (str.length()<n1 && n1 + char_index_in_bucket(pos1) - str.length()>=2*BUCKET_LENGTH)
    {
      size_t k = (n1+ char_index_in_bucket(pos1)-str.length())/BUCKET_LENGTH;
      size_t m = get_entry_index(pos1);
      
      uint64_t* entry = get_entry_array();
      for (size_t i=0; i>k; i++)
        entry[i+m+1]=0;
      
      for (size_t i=m+1; i+k<=end_entry_index(); i++)
        entry[i] = entry[i + k];

      for (size_t i=end_entry_index()-k+1; i<=end_entry_index(); i++)
        entry[i] = 0;
      
      length_ -= k*BUCKET_LENGTH;
      max_size_ -= k;
    }
    
    
    const_iterator j=str.begin();
    size_t g= 0;
    for (iterator i=begin()+pos1;g<str.length();g++, i++,j++ )
      *i = *j;

    clear_reference();
    
    return *this;
  }

public:
  SelfT& replace ( size_t pos1, size_t n1, const SelfT& str )
  {
    if (str.length() == 0)
    {
      erase(pos1, n1);
      return *this;
    }
    
    size_t new_len = length_-n1+str.length();
    
    if (is_refered())
    {
      char* p;
      size_t es = init_entry(new_len, &p);
      size_t st = BUCKET_LENGTH;
      size_t ei = 0;

      const_iterator it = begin();
      size_t i=0; 
      for (;it!= end() && i<pos1; i++, it++)
      {
        if ((st+i)%BUCKET_LENGTH==0)
          ei++;
      
        ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
          = *it;
      }
      
      it = str.begin();
      size_t g=0;
      for (; g<str.length();g++, i++, it++)
      {
        if ((st+i)%BUCKET_LENGTH==0)
          ei++;
      
        ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
          = *it;
      }
      
      it = begin()+pos1+n1;
      for (; it< end();i++, it++)
      {
        if ((st+i)%BUCKET_LENGTH==0)
          ei++;
      
        ((CharT*)(*(uint64_t*)(p + sizeof(ReferT) + ei*sizeof(CharT*))))[(st+i)%BUCKET_LENGTH]
          = *it;
      }
      
      derefer();

      entry_size_ = es;
      max_size_ = init_max_size();
      length_ = new_len;
      start_i_ = st;
      p_ = p;
      clear_reference();

      return *this;
    }
    
    if(p_ == NULL)
      return assign(str);

    return replace_(pos1, n1, str);
  }
  
  SelfT& replace ( iterator i1, iterator i2, const SelfT& str )
  {
    assert(i1<=i2);
    uint64_t i = i1 - begin();
    uint64_t j = i2 - begin();
    return replace(i, j-i+1, str);
  }

  SelfT& replace ( size_t pos1, size_t n1, const SelfT& str, size_t pos2, size_t n2 )
  {
    return replace(pos1, n1, str.substr(pos2, n2));
  }
  

  SelfT& replace ( size_t pos1, size_t n1,   const char* s, size_t n2 )
  {
    SelfT ss(s, n2);
    //ss.attach(s, n2);
    return replace(pos1, n1, ss);
  }
  
  SelfT& replace ( iterator i1, iterator i2, const CharT* s, size_t n2 )
  {
    assert(i1<=i2);
    uint64_t i = i1 - begin();
    uint64_t j = i2 - begin();
    return replace(i, j-i+1, s, n2);
  }
  

  SelfT& replace ( size_t pos1, size_t n1,   const CharT* s )
  {
    SelfT ss(s);
    //ss.attach(s);
    return replace(pos1, n1, ss);
  }
  
  SelfT& replace ( iterator i1, iterator i2, const CharT* s )
  {
    assert(i1<=i2);
    uint64_t i = i1 - begin();
    uint64_t j = i2 - begin();
    return replace(i, j-i+1, s);
  }

  SelfT& replace ( size_t pos1, size_t n1,   size_t n2, CharT c )
  {
    SelfT ss(n2, c);
    return replace(pos1, n1, ss);
  }
  
  SelfT& replace ( iterator i1, iterator i2, size_t n2, CharT c )
  {
    assert(i1<=i2);
    uint64_t i = i1 - begin();
    uint64_t j = i2 - begin();
    return replace(i, j-i+1, n2, c);
  }
  

  template<class InputIterator>
  SelfT& replace ( iterator i1, iterator i2, InputIterator j1, InputIterator j2 )
  {
    assert(i1<=i2);
    uint64_t i = i1 - begin();
    uint64_t j = i2 - begin();
    SelfT s1 = substr(i, j-i+1);
    SelfT s2 = substr(j+1);
    assign(s1);
    append<InputIterator>(j1, j2);
    append(s2);

    return *this;
  }

  size_t copy ( CharT* s, size_t n, size_t pos = 0) const
  {
    size_t j=0;
    for (const_iterator i=begin()+pos; i<end() && j<n; j++, i++)
           s[j] = *i;
         
    return j;
  }

  void swap ( SelfT& str )
  {
    SelfT s = str;
    str = *this;
    *this = s;
  }
  
  //******************String operations********************
public:
  const CharT* c_str ( ) const
  {
    return NULL;
  }

  const CharT* data() const
  {
    return NULL;
  }

  size_t find ( const SelfT& str, size_t pos = 0 ) const
  {
    size_t i = Algorithm<SelfT>::KMP(substr(pos), str);
    if (i == (size_t)-1)
      return -1;
    
    return pos + i;
  }
  
  size_t find ( const CharT* s, size_t pos, size_t n ) const
  {
    SelfT ss(s, n);
    //ss.attach(s, n);
    
    return find(ss, pos);
  }
  
  size_t find ( const CharT* s, size_t pos = 0 ) const
  {
    SelfT ss(s);
    //ss.attach(s);
    
    return find(ss, pos);
  }
  
  size_t find ( CharT c, size_t pos = 0 ) const
  {
    for (const_iterator i =begin()+pos; i<end(); i++)
      if (*i==c)
        return i-begin();
    return -1;
  }

  size_t rfind ( const SelfT& str, size_t pos = npos ) const
  {
    assert(pos == npos || pos<length_);
    
    return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), str);
  }
  
  size_t rfind ( const char* s, size_t pos, size_t n ) const
  {
    assert(pos == npos || pos<length_);
    
    SelfT ss(s, n);
    //ss.attach(s, n);
    return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
  }
  
  size_t rfind ( const char* s, size_t pos = npos ) const
  {
    assert(pos == npos || pos<length_);
    
    SelfT ss(s);
    //ss.attach(s);
    return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
  }
  
  size_t rfind ( char c, size_t pos = npos ) const
  {
    assert(pos == npos || pos<length_);
    const_iterator i = pos==npos? rbegin(): begin()+pos;
    
    for (; i!=rend(); i++)
      if (*i==c)
        return i-begin();
    return -1;
  }

//   size_t find_first_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_first_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_first_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_first_of ( char c, size_t pos = 0 ) const{}

  
//   size_t find_last_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_last_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_last_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_last_of ( char c, size_t pos = 0 ) const{}

  
//   size_t find_first_not_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_first_not_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_first_not_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_first_not_of ( char c, size_t pos = 0 ) const{}

  
//   size_t find_last_not_of ( const SelfT& str, size_t pos = 0 ) const{}
//   size_t find_last_not_of ( const char* s, size_t pos, size_t n ) const{}
//   size_t find_last_not_of ( const char* s, size_t pos = 0 ) const{}
//   size_t find_last_not_of ( char c, size_t pos = 0 ) const{}

  SelfT substr ( size_t pos = 0, size_t n = npos ) const
  {
    return SelfT(*this, pos, n);
  }

  int compare ( const SelfT& str ) const
  {
    const_iterator i=begin();
    const_iterator j=str.begin();
    size_t g = 0;
    for (; g<length() && g<str.length(); g++, j++, i++)
    {
      if (*i>*j)
        return 1;
      if (*i < *j)
        return -1;
    }

    if (g==length() && g == str.length())
      return 0;
    if (i== end())
      return -1;

    return 1;
  }
  
  int compare ( const CharT* s ) const
  {
    size_t len = getLen(s);
    
    const_iterator i=begin();
    size_t j=0;
    size_t g = 0;
    for (; g<length() && j<len; g++, j++, i++)
    {
      if (*i>s[j])
        return 1;
      if (*i<s[j])
        return -1;
    }

    if (g==length() && j==len)
      return 0;
    if (i== end())
      return -1;

    return 1;
  }
  
  int compare ( size_t pos1, size_t n1, const SelfT& str ) const
  {
    return substr(*this, pos1, n1).compare(str);
  }
  
  int compare ( size_t pos1, size_t n1, const CharT* s) const
  {
    return substr(*this, pos1, n1).compare(s);
  }
  
  int compare ( size_t pos1, size_t n1, const SelfT& str, size_t pos2, size_t n2 ) const
  {
    return substr(*this, pos1, n1).compare(str.substr(pos2, n2));
  }
  
  int compare ( size_t pos1, size_t n1, const CharT* s, size_t n2) const
  {
    SelfT str(s ,n2);
    return substr(*this, pos1, n1).compare(str);
  }

  std::vector<CharT> cast_std_vector()const
  {
    std::vector<CharT> v;
    v.resize(length_);
    for (const_iterator i=begin(); i!=end(); i++)
      v.push_back(*i);

    return v;
  }
  
  std::string cast_std_string()const
  {
    std::string s;
    s.reserve(length_);
    for (const_iterator i=begin(); i!=end(); i++)
      s +=(*i);
    return s;
  }

  
//******************Serialization********************
public:    
  
  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version)  const
  {
    ar & length_;
    ar & max_size_;
    ar & entry_size_;
    ar & start_i_;
    size_t o = -1;

    const uint64_t* en = get_const_entry_array();
    for (size_t i=0; i<entry_size_; i++)
    {
      if ((CharT*)en[i]!=NULL)
      {
        ar & i;
        ar.save_binary((char*)en[i], BUCKET_BYTES);
      }
      else
        ar & o;
    }
    
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
    derefer();
    
    ar & length_;
    ar & max_size_;
    ar & entry_size_;
    ar & start_i_;
    
    p_ = malloc_char(need_entry_bytes(entry_size_));
    uint64_t* en = get_entry_array();
    for (size_t i=0; i<entry_size_; i++)
    {
      size_t k;
      ar & k;
      if (k == (size_t)-1)
      {
        en[i] = (uint64_t)NULL;
        continue;
      }

      en[i] = (uint64_t)malloc_CharT();
      ar.load_binary((char*)en[i], BUCKET_BYTES);
      
    }
        
    clear_reference();
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

friend std::ostream& operator << (std::ostream& os, const SelfT& str)
  {
    for (const_iterator i =str.begin(); i<str.end(); i++)
      os<<(char)*i;

    return os;
  }
  
}
  ;

template <
  class CHAR_TYPE ,
  int COPY_ON_WRITE,
  uint32_t BUCKET_BYTES
  >
const size_t deque_string<CHAR_TYPE ,COPY_ON_WRITE, BUCKET_BYTES>::npos = -1;

NS_IZENELIB_AM_END
#endif

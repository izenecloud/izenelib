#ifndef AVL_STRING_HPP
#define AVL_STRING_HPP

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
  uint64_t BUCKET_BYTES = 64
  >
class bbt_string
{  
public:
  typedef CHAR_TYPE value_type;
  typedef CHAR_TYPE CharT;
  typedef uint16_t  ReferT;
  typedef bbt_string<CHAR_TYPE, COPY_ON_WRITE, BUCKET_BYTES> SelfT;
  typedef std::size_t size_t;

  enum 
    {
      BUCKET_LENGTH = BUCKET_BYTES/sizeof(CharT) -1;
      BUCKEY_SIZE = BUCKET_BYTES + sizeof(ReferT) + sizeof(void*)
    };

  
  static const size_t npos;
protected:

  struct BUCKET_
  {
    char p_[BUCKET_SIZE];

    inline BUCKET_()
    {
      *(ReferT*)(p_ + BUCKET_BYTES) = 1;
      *(LEEF_*)(p_ + BUCKET_BYTES + sizeof(ReferT)) = NULL;
    }
    
    inline CharT& operator [] (size_t i)
    {
      assert(i<BUCKET_LENGTH);
      return *((CharT*)p_ + i);
    }

    inline size_t length()const
    {
      size_t i=0;
      for (;i<BUCKET_LENGTH; i++)
        if (*((CharT*)p_ + i)=='\0')
          return i;

      assert(false);
    }

    inline void refer()
    {
      *(ReferT*)(p_ + BUCKET_BYTES) += 1;
    }

    inline bool is_refered()
    {
      if (*(ReferT*)(p_ + BUCKET_BYTES)>1)
        return true;
      return false;
    }

    inline void derefer()
    {
      if (*(ReferT*)(p_ + BUCKET_BYTES) != 0)
        *(ReferT*)(p_ + BUCKET_BYTES) -= 1;
    }

    BUCKET_* next()
    {
      return *(LEEF_**)(p_ + BUCKET_BYTES + sizeof(ReferT));
    }
    
    void set_next(BUCKET_* lf)
    {
      *(BUCKET_**)(p_ + BUCKET_BYTES + sizeof(ReferT)) = lf;
    }
    
    void clear_reference()
    {
      *(ReferT*)(p_ + BUCKET_BYTES) = 1;
    }    
    
  }
    ;

  struct INDEX_
  {
    size_t len_;
    BUKCET_* bptr_;

    inline INDEX_()
    {
      len_ = 0;
      bptr_ = NULL;
    }

    bool operator == (size_t i) const
    {
      return len_==i;
    }

    bool operator > (size_t i) const
    {
      return len_ > i;
    }

    bool operator < (size_t i) const
    {
      return len_ < i;
    }

    
    bool operator != (size_t i) const
    {
      return len_ != i;
    }

    
  }
    ;
  
public:
  class const_iterator;
  
  class iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {

  friend class const_iterator;
  public:
    iterator(SelfT* obj=NULL, CharT* p=NULL) : p_(p), obj_(obj) {}
    ~iterator() {}

    // The assignment and relational operators are straightforward
    iterator& operator = (const iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }

    bool operator==(const iterator& other)const
    {
      return(p_ == other.p_);
    }

    bool operator!=(const iterator& other)const
    {
      return(p_ != other.p_);
    }

    bool operator < (const iterator& other)const
    {
      return(p_ < other.p_);
    }
    
    bool operator > (const iterator& other)const
    {
      return(p_ > other.p_);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    iterator& operator++()
    {
      if (p_ != NULL)
      {
        p_++;
        
      }
      return(*this);
    }

    iterator operator++(int)
    {
      iterator tmp(*this);
      p_++;
      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    iterator& operator--()
    {
      if (p_ != NULL)
      {
        p_--;
        
      }
      return(*this);
    }

    iterator operator--(int)
    {
      iterator tmp(*this);
      p_--;
      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    CharT& operator*()
    {
      if (obj_->is_refered())
      {
        uint64_t i = p_ - obj_->str_;
        (*obj_).assign_self();
        p_ = obj_->str_ + i;
      }
      
      return(*p_);
    }
    
    iterator& operator + (size_t i)
    {
      p_ += i;
      return *this;
    }

    iterator& operator - (size_t i)
    {
      p_ -= i;
      return *this;
    }

    uint64_t operator - (const CharT* s) const
    {
      return p_ -s;
    }
    
  protected:
    CharT* p_;
    SelfT* obj_;
  };

  
  class const_iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {
  public:
    const_iterator(const CharT* p=NULL) : p_(p) {}
    const_iterator(const iterator& other)
    {
      p_ = other.p_;
    }
    
    ~const_iterator() {}

    // The assignment and relational operators are straightforward
    const_iterator& operator=(const const_iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }
    
    // The assignment and relational operators are straightforward
    const_iterator& operator = (const iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }

    bool operator==(const const_iterator& other) const
    {
      return(p_ == other.p_);
    }

    bool operator < (const const_iterator& other) const
    {
      return(p_ < other.p_);
    }
    
    bool operator > (const const_iterator& other) const
    {
      return(p_ > other.p_);
    }

    bool operator!=(const const_iterator& other) const
    {
      return(p_ != other.p_);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator++()
    {
      if (p_ != NULL)
      {
        p_++;
        
      }
      return(*this);
    }

    const_iterator operator++(int)
    {
      const_iterator tmp(*this);
      p_++;
      return(tmp);
    }
    
    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator--()
    {
      if (p_ != NULL)
      {
        p_--;
        
      }
      return(*this);
    }

    const_iterator operator--(int)
    {
      const_iterator tmp(*this);
      p_--;
      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const CharT& operator*() const
    {
      return(*p_);
    }
    
    uint64_t operator - (const CharT* s) const
    {
      return p_ -s;
    }
    
    iterator& operator + (size_t i)
    {
      p_ += i;
      return *this;
    }

    iterator& operator - (size_t i)
    {
      p_ -= i;
      return *this;
    }

  private:
    const CharT* p_;
  };


  class const_reverse_iterator;
  class reverse_iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {
    
  friend class const_reverse_iterator;
  public:
    reverse_iterator(SelfT* obj=NULL, CharT* p=NULL) : p_(p), obj_(obj) {}
    ~reverse_iterator() {}

    // The assignment and relational operators are straightforward
    reverse_iterator& operator=(const reverse_iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }

    bool operator==(const reverse_iterator& other)const
    {
      return(p_ == other.p_);
    }

    bool operator!=(const reverse_iterator& other)const
    {
      return(p_ != other.p_);
    }
    
    bool operator > (const reverse_iterator& other)const
    {
      return(p_ < other.p_);
    }

    bool operator < (const reverse_iterator& other)const
    {
      return(p_ > other.p_);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    reverse_iterator& operator++()
    {
      if (p_ != NULL)
      {
        p_--;
        
      }
      return(*this);
    }

    reverse_iterator operator++(int)
    {
      reverse_iterator tmp(*this);
      p_--;
      return(tmp);
    }
    
    // Update my state such that I refer to the next element in the
    // SQueue.
    reverse_iterator& operator--()
    {
      if (p_ != NULL)
      {
        p_++;
        
      }
      return(*this);
    }

    reverse_iterator operator--(int)
    {
      reverse_iterator tmp(*this);
      p_++;
      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    CharT& operator*()
    {
      if (obj_->is_refered())
      {
        uint64_t i = p_ - obj_->str_;
        (*obj_).assign_self();
        p_ = obj_->str_ + i;
      }

      return *p_;
    }
        
    iterator& operator + (size_t i)
    {
      p_ -= i;
      return *this;
    }

    iterator& operator - (size_t i)
    {
      p_ += i;
      return *this;
    }

    uint64_t operator - (const CharT* s)
    {
      return p_ -s;
    }

  private:
    CharT* p_;
    SelfT* obj_;
  };

  
  class const_reverse_iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {
  public:
    const_reverse_iterator(const CharT* p=NULL) : p_(p) {}
    const_reverse_iterator(const reverse_iterator& other)
    {
      p_ = other.p_;
    }
    
    ~const_reverse_iterator() {}

    // The assignment and relational operators are straightforward
    const_reverse_iterator& operator=(const const_reverse_iterator& other)
    {
      p_ = other.p_;
      return(*this);
    }

    bool operator==(const const_reverse_iterator& other) const
    {
      return(p_ == other.p_);
    }

    bool operator!=(const const_reverse_iterator& other) const
    {
      return(p_ != other.p_);
    }
    
    bool operator > (const const_reverse_iterator& other)const
    {
      return(p_ < other.p_);
    }

    bool operator < (const const_reverse_iterator& other)const
    {
      return(p_ > other.p_);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_reverse_iterator& operator++()
    {
      if (p_ != NULL)
      {
        p_--;
        
      }
      return(*this);
    }

    const_reverse_iterator operator++(int)
    {
      const_reverse_iterator tmp(*this);
      p_--;
      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_reverse_iterator& operator--()
    {
      if (p_ != NULL)
      {
        p_++;
        
      }
      return(*this);
    }

    const_reverse_iterator operator--(int)
    {
      const_reverse_iterator tmp(*this);
      p_++;
      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const CharT& operator*() const
    {
      return(*p_);
    }

        
    iterator& operator + (size_t i)
    {
      p_ += i;
      return *this;
    }

    iterator& operator - (size_t i)
    {
      p_ -= i;
      return *this;
    }

    uint64_t operator - (const CharT* s)
    {
      return p_ -s;
    }

  private:
    const CharT* p_;
  };

  
protected:
  BUCKET_* head_;
  INDEX_* indice_;
  size_t idx_len_;
  size_t idx_size_;
  size_t length_;

  inline bool refer()
  {
    if (!COPY_ON_WRITE)
      return false;

    //lock
    BUCKET_* b = head_;
    while (b !=NULL)
    {
      b->refer();
      b = b->next();
    }

    return true;
  }

  void derefer()
  {
    hlfree(indice_);
    indice_ = NULL;
    idx_len_ = length_ = 0;
    
    if (!COPY_ON_WRITE)
    {
      BUCKET_* b = head_;
      while (b !=NULL)
      {
        BUCKET_* t = b->next();
        hlfree(b);
        b = t;
      }

      head_ = NULL;
      return;
    }
    
    BUCKET_* b = head_;
    BUCKET_* last = NULL;
    while (b !=NULL)
    {
      BUCKET_* t = b->next();
      //lock
      b->derefer();
      if (!b->is_refered())
      {
        hlfree(b);
        if (last != NULL)
        {
          last->set_next(NULL);
          last = NULL;
        }
      }
      else
        last = b;
      
      b = t;
    }

    head_ = NULL;
  }

  void clear_reference(BUCKET_* b)
  {
    if (b == NULL)
      return;

    clear_reference(lf->next());
    lf->clear_reference();
  }
  
  void clear_reference()
  {     
    if (!COPY_ON_WRITE)
      return ;

        
    BUCKET_* b = head_;
    while (b !=NULL)
    {
      //lock
      b->clear_reference();
      b = b->next();
    }

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

  void duplicate(INDEX_** indice, BUCKET_** bu) const
  {
    *indice = (INDEX_*)hlmalloc(idx_len_*sizeof(INDEX_));
    memcpy (*indice, indice_, idx_len_*sizeof(INDEX_));
    if (COPY_ON_WRITE)
    {
      *bu = head_;
      return;
    }

    size_t i = 0;
    BUCKET_* b1 = head_;
    
    if (b1!=NULL)
    {
      *bu = (BUCKET_*)hlmalloc(BUCKET_SIZE);
      memcpy(*bu, b1, BUCKET_SIZE);
      (*indice)[i].bptr_ = *bu;
      i++;
      b1 = b1->next();
    }
    
    BUCKET_* last = *bu;
    
    while (b1!=NULL)
    {
      BUCKET_* t = (BUCKET_*)hlmalloc(BUCKET_SIZE);
      memcpy(t, b1, BUCKET_SIZE);
      (*indice)[i].bptr_ = t;
      last->set_next(t);
      last = b1;
      
      b1 = b1->next();
      i++;
    }

    if (head_!=NULL)
      last->set_next(NULL);
    
    
  }
  
  size_t binary_search(size_t i, size_t start, size_t end)
  {
    if (end == start)
      return end;

    size_t mid = (start+end)/2;
    if (i+1 > indice[mid])
      return binary_search(i, mid+1, end);
    else if (i+1 == indice[mid])
      return mid;
    else if (i+1 < indice[mid])
      return binary_search(i, start, mid-1);
  }
  
  size_t duplicate(size_t start, size_t n, INDEX_** indice, BUCKET_** bu) const
  {
    assert(start + n < length_);
    
    size_t i = binary_search(start, 0, idx_len_-1);
    size_t last_i = i==0? 0: indice_[i-1].len_;
    if (i-last_i==0)
    {
      *bu = indice_[i].bptr_;
      *bu->refer();
    }
    else
    {
      *bu = (BUCKET_*)hlmalloc(BUCKET_SIZE);
      memcpy(*bu, indice_[i].bptr_->p_ + (i-last_i)*sizeof(CharT), BUCKET_SIZE-(i-last_i)*sizeof(CharT));
      *bu->clear_reference();
      *bu->set_next(indice_[i].bptr_->next());
    }
    
    size_t e = binary_search(start+n-1, i, idx_len_-1);
    *indice = (INDEX_*)hlmalloc((e-i+1)*sizeof(INDEX_));
    memcpy (*indice, indice_ + i, (e-i+1)*sizeof(INDEX_));
    (*indice)[0].bptr_ = *bu;
    BUCKET_* b = *bu->next();
    size_t ii = 1;
    i++;

    while (i<=e)
    {
      assert(b!=NULL);

      if (COPY_ON_WRITE)
      {
        (*indice)[ii].bptr_->refer();
      }
      else
      {
        BUCKET_* t = (BUCKET_*)hlmalloc(BUCKET_SIZE);
        memcpy(t, indice_[i].bptr_, BUCKET_SIZE);
        t->clear_reference();
        (*indice)[ii].bptr_ = t;
        (*indice)[ii-1].bptr_->set_next(t);
      }

      b = b->next();
      i++;
      ii++;
    }

    return e-i+1;
  }

public:
  explicit bbt_string()
  {
    idx_len_ = length_ = 0;
    head_ = (BUCKET_*)hlmalloc(sizeof(BUCKET_));
    indice_ = (INDEX_*)hlmalloc(BUCKET_LENGTH*sizeof(INDEX_));
    memset(indice_, BUCKET_LENGTH*sizeof(INDEX_),0);
    idx_size_ = BUCKET_LENGTH;
  }

  bbt_string ( const SelfT& str )
  //    :p_(NULL), is_attached_(false),str_(NULL),length_(0),max_size_(0)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(str);
  }
  
  bbt_string ( const SelfT& str, size_t pos, size_t n = npos )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(str, pos, n);
  }
  
  bbt_string ( const CharT * s, size_t n )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(s, n);
  }
  
  bbt_string ( const CharT * s )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(s);
  }
  
  bbt_string ( size_t n, CharT c )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(n, c);
  }

  bbt_string (const std::vector<CharT>& v)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(v);
  }

  bbt_string (const std::string& str)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(str);
  }
  

  template<class InputIterator> bbt_string (InputIterator begin, InputIterator end)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(begin(), end);
  }

  virtual ~bbt_string()
  {
    derefer();
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
    return const_iterator(str_);
  }

  iterator begin()
  {
    return iterator(this, str_);
  }

  const_iterator end() const
  {
    return const_iterator(str_+length_);
  }

  iterator end()
  {
    return iterator(this, str_+length_);
  }

  reverse_iterator rbegin()
  {
    return reverse_iterator(this, str_+length_-1);
  }

  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(str_+length_-1);
  }

  reverse_iterator rend()
  {
    return reverse_iterator(this, str_-1);
  }

  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(str_-1);
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
    return idx_len_ * BUCKET_LENGTH;
  }

  void resize (size_t n, CharT c)
  {
    if (n == length_ && n == max_size())
      return;

    
    idx_len_ = n%BUCKET_LENGTH==0? n/BUCKET_LENGTH: n/BUCKET_LENGTH+1;
    if (n > idx_size_*BUCKET_LENGTH)
    {
      idx_size_ = idx_len_;
      indice_ = (INDEX_*)hlreallocate(indice_, idx_size_);
    }

    for (size_t i=0; i<idx_len_; i++)
    {
      if (indice_[i].bptr_ != NULL )
      {
        if(indice_[i].bptr_->is_refered())
        {
          BUCKET_* t= indice_[i].bptr_;
          indice_[i].bptr_ = (BUCKET_*)hlmalloc(BUCKET_SIZE);
          memcpy(indice_[i].bptr_, t, BUCKET_SIZE);
          indice_[i].bptr_->clear_reference();
          t->derefer();
        }

        if (i==0 && indice_[i].len_==0
            || i!=0 && indice_[i-1].len_ == indice_[i].len_)
        {
          for (size_t k=0; k<BUCKET_LENGTH; k++)
            indice_[i].bptr_->p_[i] = c;
          indice_[i].bptr_->p_[BUCKET_LENGTH] = '\0';
          indice_[i].len_ = BUCKET_LENGTH + (i==0?0: indice_[i-1].len_);
          length_ += BUCKET_LENGTH;
        }
        
      }
      else
      {
        indice_[i].bptr_ = (BUCKET_*)hlmalloc(BUCKET_SIZE);
        indice_[i].bptr_->clear_reference();
        for (size_t k=0; k<BUCKET_LENGTH; k++)
            indice_[i].bptr_->p_[i] = c;
          indice_[i].bptr_->p_[BUCKET_LENGTH] = '\0';
        indice_[i].len_ = BUCKET_LENGTH + (i!=0? indice_[i].len_: 0);
        length_ += BUCKET_LENGTH;
      }

      if (i!=0)
        indice_[i-1].bptr_->set_next(indice_[i].bptr_);
    }
  }

  void resize ( size_t n )
  {
    if (n == max_size())
      return;

    idx_len_ = n%BUCKET_LENGTH==0? n/BUCKET_LENGTH: n/BUCKET_LENGTH+1;
    if (n > idx_size_*BUCKET_LENGTH)
    {
      idx_size_ = idx_len_;
      indice_ = (INDEX_*)hlreallocate(indice_, idx_size_);
    }

    for (size_t i=0; i<idx_len_; i++)
    {
      if (indice_[i].bptr_ != NULL )
      {
        if(indice_[i].bptr_->is_refered())
        {
          BUCKET_* t= indice_[i].bptr_;
          indice_[i].bptr_ = (BUCKET_*)hlmalloc(BUCKET_SIZE);
          memcpy(indice_[i].bptr_, t, BUCKET_SIZE);
          indice_[i].bptr_->clear_reference();
          t->derefer();
        }
      }
      else
      {
        indice_[i].bptr_ = (BUCKET_*)hlmalloc(BUCKET_SIZE);
        indice_[i].bptr_->clear_reference();
        indice_[i].len_ = BUCKET_LENGTH + (i!=0? indice_[i].len_: 0);
      }

      if (i!=0)
        indice_[i-1].bptr_->set_next(indice_[i].bptr_);
    }

    length_ = n;

  }

  size_t capacity ( ) const
  {
    return idx_len_ * sizeof(CharT) * BUCKET_LENGTH;
  }

  void reserve ( size_t res_arg=0 )
  {
    if (res_arg<= capacity())
      return;

  }

  void clear()
  {
    derefer();
    length_ = max_size_ = 0;
    str_ = NULL;
    is_attached_ = false;
  }

  bool empty ( ) const
  {
    return (length_==0);
  }

  //******************Element access********************
public:
  const CharT& operator[] ( size_t pos ) const
  {
    assert(pos<length_);
    return str_[pos];
  }
  
  CharT& operator[] ( size_t pos )
  {
    assert(pos<length_);
    assign_self();
    return str_[pos];
    
  }

  const CharT& at ( size_t pos ) const
  {
    assert(pos<length_);
    return str_[pos];
  }
  
  CharT& at ( size_t pos )
  {
    assert(pos<length_);
    assign_self();
    return str_[pos];
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

    size_t new_len = (size_t)((length_ + str.length()) * append_rate_);
    
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(new_len));
      memcpy(p + sizeof(ReferT), str_, size());
      str_ = (CharT*)(p+sizeof (ReferT));
      memcpy(str_ + length_, str.str_, str.size());
      
      length_ = length_ + str.length();
      max_size_ = new_len+1;
      str_[length_] = '\0';
      
      derefer();
      p_ = p;
      clear_reference();

      return *this;
    }

    if (str.length()+length_+1>max_size_)
    {
      if (p_!=NULL)
        p_  = (char*)hlrealloc(p_, get_total_size(new_len));
      else
        p_  = (char*)hlmalloc(get_total_size(new_len));
      
      str_ = (CharT*)(p_ + sizeof (ReferT));
      max_size_ = new_len+1;
    }
    
    memcpy(str_ + length_, str.str_, str.size());
    length_ +=  str.length();
    str_[length_] = '\0';
    
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
    
    size_t new_len = (size_t)((length_ + n)* append_rate_);
    
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(new_len));
      memcpy(p + sizeof(ReferT), str_, size());
      str_ = (CharT*)(p+sizeof (ReferT));
      memcpy(str_ + length_, s, n * sizeof (CharT));

      length_ = length_ + n;
      max_size_ = new_len+1;
      str_[length_] = '\0';
      
      derefer();
      p_ = p;
      clear_reference();

      return *this;
    }

    if ( n + length_+1 > max_size_)
    {
      if (p_!=NULL)
        p_  = (char*)hlrealloc(p_, get_total_size(new_len));
      else
        p_  = (char*)hlmalloc(get_total_size(new_len));
      str_ = (CharT*)(p_ + sizeof (ReferT));
      max_size_ = new_len+1;
    }
    
    memcpy(str_ + length_, s, n);
    length_ += n;
    str_[length_] = '\0';
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
    
    size_t new_len = (size_t)((length_ + n)* append_rate_);
    
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(new_len));
      memcpy(p + sizeof(ReferT), str_, size());
      str_ = (CharT*)(p+sizeof (ReferT));
      for (size_t i=0; i<n; i++)
        str_[length_+i] = c;

      length_ = length_ + n;
      max_size_ = new_len+1;
      str_[length_] = '\0';
      
      derefer();
      p_ = p;
      clear_reference();

      return *this;
    }

    if ( n + length_ > max_size_)
    {
      if (p_!=NULL)
        p_  = (char*)hlrealloc(p_, get_total_size(new_len));
      else
        p_  = (char*)hlmalloc(get_total_size(new_len));
      
      str_ = (CharT*)(p_ + sizeof (ReferT));
    }
    
    for (size_t i=0; i<n; i++)
        str_[length_+i] = c;
    
    length_ += n;
    max_size_ = new_len+1;
    str_[length_] = '\0';
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

  SelfT& assign ( const SelfT& str )
  {
    derefer();

    str.duplicate(&indice_, &head_);
    refer();
  
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
    
    idx_len_ = idx_size_ = duplicate(pos, length_, &indice_, &head_);
    
    return *this;
  }
  
  SelfT& assign ( const CharT* s, size_t n )
  {
    derefer();

    size_t i=0;
    size_t j=0;
    length_ = n;

    head_ = (BUCKET_*)hlmalloc(BUCKET_SIZE);
    indice_[j].len_ = n>BUCKET_LENGTH? BUCKET_LENGTH:n;
    memcpy(head_->p_, s, indice_[j].len_*sizeof(CharT) );
    i+= indice_[j].len_;

    idx_size_ = n/BUCKET_LENGTH+1;
    indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
    indice_[j].bptr_ = head_;
    head_->p_[i] = '\0';
    j++;

    while (i < n)
    {
      BUCKET_* t = (BUCKET_*)hlmalloc(BUCKET_SIZE);
      indice_[j].len_ = n-i>BUCKET_LENGTH? BUCKET_LENGTH:n-i;
      memcpy(t->p_, s+i, indice_[j].len_*sizeof(CharT) );
      t->p_[indice_[j].len_] = '\0';
      t->clear_reference();
      t->set_next(NULL);
      
      indice_[j-1].bptr_->set_next(t);
      indice_[j].bptr_ = t;
      indice_[j].len_ += indice_[j-1].len_;

      i+= BUCKET_LENGTH;
      j++;
    }

    idx_len_ = j;

    return *this;
  }

  SelfT& assign ( const CharT* s )
  {
    assert (s !=NULL);
    derefer();
    
    length_ = getLen(s);

    return assign(s, n);
  }

  SelfT& assign ( size_t n, CharT c )
  {
    derefer();
    
    size_t i=0;
    size_t j=0;
    length_ = n;

    head_ = (BUCKET_*)hlmalloc(BUCKET_SIZE);
    indice_[j].len_ = n>BUCKET_LENGTH? BUCKET_LENGTH:n;
    for (size_t k=0; k<indice_[j].len_; k++)
      head_->p_[k] = c;
    i+= indice_[j].len_;

    idx_size_ = n/BUCKET_LENGTH+1;
    indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
    indice_[j].bptr_ = head_;
    head_->p_[i] = '\0';
    j++;

    while (i < n)
    {
      BUCKET_* t = (BUCKET_*)hlmalloc(BUCKET_SIZE);
      indice_[j].len_ = n-i>BUCKET_LENGTH? BUCKET_LENGTH:n-i;
      for (size_t k=0; k<indice_[j].len_; k++)
        t_->p_[k] = c;
      
      t->p_[indice_[j].len_] = '\0';
      t->clear_reference();
      t->set_next(NULL);
      
      indice_[j-1].bptr_->set_next(t);
      indice_[j].bptr_ = t;
      indice_[j].len_ += indice_[j-1].len_;

      i+= BUCKET_LENGTH;
      j++;
    }

    idx_len_ = j;
    
    return *this;
  }

  SelfT& assign (const std::vector<CharT>& v)
  {
    return assign(v.data(), v.size());
  }

  SelfT& assign (const std::string& str)
  {
    return assign(str.data(), str.length())
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

    size_t new_len = (size_t)((length_ + str.length()) * append_rate_);
    
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(new_len));
      memcpy(p + sizeof(ReferT), str_, (pos1+1)*sizeof(CharT));
      memcpy(p + sizeof(ReferT) + (pos1+1)*sizeof(CharT), str.str_, str.size());
      memcpy(p + sizeof(ReferT) + (pos1+1 + str.length())*sizeof(CharT), str_+pos1+1, length_-pos1-1);
      str_ = (CharT*)(p+sizeof (ReferT));
      

      length_ = length_ + str.length();
      max_size_ = new_len+1;
      str_[length_] = '\0';
      
      derefer();
      p_ = p;
      clear_reference();

      return *this;
    }

    if (p_ == NULL)
      return assign(str);
    
    if (new_len+1 > max_size_)
    {
      max_size_ = new_len+1;
    }

    char* pp = (char*)hlmalloc(get_total_size(new_len));
    
    memcpy(pp + sizeof(ReferT), str_, (pos1+1)*sizeof(CharT));
    
    memcpy(pp + sizeof(ReferT) + (pos1+1)*sizeof(CharT), str.str_, str.size());
    memcpy(pp + sizeof(ReferT) + (pos1+1)*sizeof(CharT)+str.size(), str_+pos1+1, (length_-pos1-1)*sizeof(CharT));
    
    length_ += str.length();
    hlfree(p_);
    p_ = pp;
    str_ = (CharT*)(p_ + sizeof (ReferT));
    str_[length_] = '\0';
    is_attached_ = false;
    clear_reference();
    
    return *this;
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
    uint64_t i = p - str_;
    assign_self();
    insert(i, 1, c);
    return iterator(str_+i+1);
  }
  
  void insert ( iterator p, size_t n, CharT c )
  {
    uint64_t i = p - str_;
    assign_self();
    insert(i, n, c);
    return iterator(str_+i+1);
  }
  
  template<class InputIterator>
  void insert( iterator p, InputIterator first, InputIterator last )
  {
    uint64_t i = p - str_;
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

    if (pos ==0 && n>= length_)
    {
      length_ = 0;
      str_[0]= '\0';
      
      return *this;
    }

    assert(pos<length_);
    if (n >= length_ - pos)
    {
      length_ = pos;
      str_[length_]= '\0';
      return *this;
    }

      
    for (size_t i=0; i<length_-n-pos; i++)
        str_[pos+i] = str_[pos+n+i];

    length_ -= n;
    str_[length_]= '\0';
    clear_reference();

    return *this;
  }
  
  iterator erase ( iterator position )
  {
    uint64_t i = position - str_;
    erase(i, 1);
    return iterator(str_+i);
  }
  
  iterator erase ( iterator first, iterator last )
  {
    uint64_t i = first - str_;
    uint64_t j = last - str_;
    erase(i, j-i);
    return iterator(str_+i);
  }
  

  SelfT& replace ( size_t pos1, size_t n1,   const SelfT& str )
  {
    if (str.length() == 0)
    {
      erase(pos1, n1);
      return *this;
    }

    size_t new_len = length_-n1+str.length();
    
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(new_len));
      memcpy(p + sizeof(ReferT), str_, pos1*sizeof(CharT));
      memcpy(p + sizeof(ReferT) + pos1*sizeof(CharT), str.str_, str.size());
      memcpy(p + sizeof(ReferT) + (pos1 + str.length())*sizeof(CharT), str_+pos1+n1, length_-pos1-n1);
      str_ = (CharT*)(p+sizeof (ReferT));
      

      length_ = length_ -n1 + str.length();
      max_size_ = new_len+1;
      str_[length_]= '\0';
      
      derefer();
      p_ = p;
      clear_reference();

      return *this;
    }

    if (p_==NULL)
      return *this;

    memcpy(str_ + pos1, str.str_, str.size());
    for (size_t i=0; i<length_-n1 -pos1; i++)
      str_[pos1+str.length() + i] = str_[pos1+n1+i];
    
    length_ = new_len;
    str_[length_]= '\0';
    clear_reference();
    
    return *this;
  }
  
  SelfT& replace ( iterator i1, iterator i2, const SelfT& str )
  {
    assert(i1<=i2);
    uint64_t i = i1 - str_;
    uint64_t j = i2 - str_;
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
    uint64_t i = i1 - str_;
    uint64_t j = i2 - str_;
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
    uint64_t i = i1 - str_;
    uint64_t j = i2 - str_;
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
    uint64_t i = i1 - str_;
    uint64_t j = i2 - str_;
    return replace(i, j-i+1, n2, c);
  }
  

  template<class InputIterator>
  SelfT& replace ( iterator i1, iterator i2, InputIterator j1, InputIterator j2 )
  {
    assert(i1<=i2);
    uint64_t i = i1 - str_;
    uint64_t j = i2 - str_;
    SelfT s1 = substr(i, j-i+1);
    SelfT s2 = substr(j+1);
    assign(s1);
    append<InputIterator>(j1, j2);
    append(s2);

    return *this;
  }

  size_t copy ( CharT* s, size_t n, size_t pos = 0) const
  {
    size_t len = pos+n>length_? length_-pos : n;
    memcpy(s, str_+pos, len*sizeof(CharT));

    return len;
  }

  void swap ( SelfT& str )
  {
    SelfT s = str;
    str = *this;
    *this = s;
  }
  
  SelfT& attach(CharT* s)
  {
    derefer();
    if (COPY_ON_WRITE)
    {
      is_attached_ = true;
      max_size_ = length_ = getLen(s);
      p_ = NULL;
      str_ = s;
    }
    else
    {
      max_size_ = length_ = getLen(s);
      p_ = (char*)hlmalloc(get_total_size(length_));
      str_ = (CharT*)(p_ + sizeof (ReferT));
      is_attached_ = false;
      clear_reference();
      memcpy(str_, s, length_*sizeof(CharT));
    }

    return *this;
  }

  SelfT& attach(CharT* s, size_t n)
  {    
    derefer();
    if (COPY_ON_WRITE)
    {
      is_attached_ = true;
      max_size_ = length_ = n;
      p_ = NULL;
      str_ = s;
    }
    else
    {
      max_size_ = length_ = n;
      p_ = (char*)hlmalloc(get_total_size(length_));
      str_ = (CharT*)(p_ + sizeof (ReferT));
      is_attached_ = false;
      clear_reference();
      memcpy(str_, s, length_*sizeof(CharT));      
    }

    return *this;
  }

  //******************String operations********************
public:
  const CharT* c_str ( ) const
  {
    assert(length_<=max_size_);
    
    if (str_[length_] == '\0')
      return str_;

    assert(is_refered());
    assign_self();
    return str_;
  }

  const CharT* data() const
  {
    return str_;
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
    for (size_t i =pos; i<length_; i++)
      if (str_[i]==c)
        return i;
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
  
  size_t rfind ( const CharT* s, size_t pos = npos ) const
  {
    assert(pos == npos || pos<length_);
    
    SelfT ss(s);
    //ss.attach(s);
    return Algorithm<SelfT>::rKMP(substr(0, (pos!=npos? pos+1: npos)), ss);
  }
  
  size_t rfind ( CharT c, size_t pos = npos ) const
  {
    assert(pos == npos || pos<length_);
    for (size_t i = (pos!=npos? pos+1: npos); i!=-1; i--)
      if (str_[i] == c)
        return i;
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
    size_t i=0;
    for (; i<length_ && i<str.length(); i++)
    {
      if (str_[i]>str[i])
        return 1;
      if (str_[i]<str[i])
        return -1;
    }

    if (i==length_ && i==str.length())
      return 0;
    if (i== length_)
      return -1;

    return 1;
  }
  
  int compare ( const CharT* s ) const
  {
    size_t len = getLen(s);
    
    size_t i=0;
    for (; i<length_ && i<len; i++)
    {
      if (str_[i]>s[i])
        return 1;
      if (str_[i]<s[i])
        return -1;
    }

    if (i==length_ && i==len)
      return 0;
    if (i== length_)
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
    SelfT str;
    str.attach(s, n2);
    return substr(*this, pos1, n1).compare(str);
  }

  std::vector<CharT> cast_std_vector()
  {
    std::vector<CharT> v;
    v.resize(length_);
    for (size_t i =0; i<length_; i++)
      v.push_back(str_[i]);

    return v;
  }
  
  std::string cast_std_string()
  {
    return std::string((char*)str_, size());

  }

  
//******************Serialization********************
public:    
  
  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version)  const
  {
    ar & length_;
    ar & max_size_;
    
    ar.save_binary(str_, max_size_*sizeof(CharT));
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
    derefer();
    
    ar & length_;
    ar & max_size_;
    
    p_ = (char*)hlmalloc(get_total_size( max_size_-1));
    str_ = (CharT*)(p_ + sizeof (ReferT));
    is_attached_ = false;
    
    ar.load_binary(str_, max_size_);
    clear_reference();
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

friend std::ostream& operator << (std::ostream& os, const SelfT& str)
  {
    for (size_t i =0; i<str.length_; i++)
      os<<str.str_[i];

    return os;
  }
  
}
  ;

template <
  class CHAR_TYPE ,
  int COPY_ON_WRITE,
  uint8_t APPEND_RATE
  >
const size_t bbt_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::npos = -1;

NS_IZENELIB_AM_END
#endif

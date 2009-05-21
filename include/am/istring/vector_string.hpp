#ifndef VECTOR_STRING_HPP
#define VECTOR_STRING_HPP

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
  uint8_t APPEND_RATE = 50
  >
class vector_string
{
public:
  typedef CHAR_TYPE value_type;
  typedef CHAR_TYPE CharT;
  typedef uint16_t  ReferT;
  typedef vector_string<CHAR_TYPE, COPY_ON_WRITE, APPEND_RATE> SelfT;
  typedef std::size_t size_t;
  
  static const size_t npos;

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
  char* p_;
  CharT* str_;
  bool is_attached_;
  size_t length_;
  size_t max_size_;

  static const float append_rate_;

  inline bool refer()
  {
    if (!COPY_ON_WRITE)
      return false;
    //lock
    if (is_attached_)
      return true;
    
    if (p_!=NULL)
      (*(ReferT*)p_)++;

    return true;
  }
  
  void derefer()
  { 
    if (!COPY_ON_WRITE)
    {
      if (p_!=NULL) hlfree(p_);
      return ;
    }

    //lock
    if (is_attached_)
    {
      is_attached_ =false;
      return;
    }

    if (p_!=NULL)
      if (*(ReferT*)p_ > 0)
      (*(ReferT*)p_)--;

    if (p_!=NULL)
      if (*(ReferT*)p_== 0)
      {
        //std::cout<<"derefer\n";
        hlfree(p_);
        p_ = NULL;
      }
    
  }

  inline void clear_reference()
  {     
    if (!COPY_ON_WRITE)
      return ;

    //lock
    if (is_attached_)
      return;
    
    if (p_!=NULL)
      (*(ReferT*)p_) = 1;
  }

  inline bool is_refered() const
  {
    if (!COPY_ON_WRITE)
      return false;
    
    //lock
    if (is_attached_)
      return true;

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

    char* p = (char*)hlmalloc(get_total_size(length_));
    memcpy(p + sizeof (ReferT), str_, capacity());
    str_ = (CharT*)(p+sizeof (ReferT));
    max_size_ = length_+1;
    str_[length_] = '\0';
    
    derefer();
    p_ = p;
    is_attached_ = false;
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

  inline size_t get_total_size(size_t str_len)
  {
    return (str_len+1)*sizeof(CharT) + sizeof(ReferT);
  }
  
public:
  explicit vector_string()
  {
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
  }

  vector_string ( const SelfT& str )
  //    :p_(NULL), is_attached_(false),str_(NULL),length_(0),max_size_(0)
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(str);
  }
  
  vector_string ( const SelfT& str, size_t pos, size_t n = npos )
  {
        
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(str, pos, n);
  }
  
  vector_string ( const CharT * s, size_t n )
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(s, n);
  }
  
  vector_string ( const CharT * s )
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(s);
  }
  
  vector_string ( size_t n, CharT c )
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(n, c);
  }

  vector_string (const std::vector<CharT>& v)
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(v);
  }

  vector_string (const std::string& str)
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(str);
  }
  

  template<class InputIterator> vector_string (InputIterator begin, InputIterator end)
  {    
    p_ = NULL;
    str_ = NULL;
    is_attached_ = false;
    length_ = 0;
    max_size_ = 0;
    assign(begin(), end);
  }

  virtual ~vector_string()
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
    return (max_size_);
  }

  void resize (size_t n, CharT c)
  {
    assert (max_size_>=length_);
    
    if (n == length_ && n+1 == max_size_)
      return;
    
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(n));
      if (max_size_ <= n)
        memcpy(p + sizeof (ReferT), str_, capacity());
      else
        memcpy(p + sizeof (ReferT), str_, n*sizeof(CharT));
      str_ = (CharT*)(p+sizeof (ReferT));
    
      for (size_t i=length_; i<n; i++)
        str_[i] = c;
      
      derefer();
      p_ = p;
      str_[n] = '\0';
      clear_reference();
      is_attached_ = false;
    }
    else
    {
      if (n!=max_size_)
      {
        p_ = (char*)hlrealloc(p_, get_total_size(n));
        str_ = (CharT*)(p_ + sizeof (ReferT));
      }
      for (size_t i=length_; i<n; i++)
        str_[i] = c;
      str_[n] = '\0';
    }

    length_ = n;
    max_size_ = n+1;
  }

  void resize ( size_t n )
  {
    assert (max_size_>=length_);
    
    if (n == max_size_)
    {
      return;
    }

    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(n));
      if (max_size_ <= n)
        memcpy(p +sizeof (ReferT), str_, capacity());
      else
        memcpy(p +sizeof (ReferT), str_, n*sizeof(CharT));
      str_ = (CharT*)(p+sizeof (ReferT));
    
      max_size_ = n;
      derefer();
      p_ = p;
      str_[n]='\0';
      clear_reference();
      is_attached_ = false;
    }
    else
    {
      p_ = (char*)hlrealloc(p_, get_total_size(n));
      str_ = (CharT*)(p_ + sizeof (ReferT));
      str_[n]='\0';
    }
    
    length_ = n;
    max_size_ = n+1;
  }

  size_t capacity ( ) const
  {
    return max_size_ * sizeof(CharT);
  }

  void reserve ( size_t res_arg=0 )
  {
    if (res_arg<= capacity())
      return;

    max_size_ = res_arg/sizeof(CharT)+1;
    if (is_refered())
    {
      char* p = (char*)hlmalloc(get_total_size(max_size_-1));
      memcpy(p + sizeof(ReferT), str_, capacity());
      str_ = (CharT*)(p+sizeof (ReferT));
    
      derefer();
      p_ = p;
      is_attached_ = false;
    }
    else
    {
      if (p_==NULL)
        p_ = (char*)hlmalloc(get_total_size(max_size_-1));
      else
        p_ = (char*)hlrealloc(p_, get_total_size(max_size_-1));
      str_ = (CharT*)(p_+sizeof (ReferT));
    }

    if (max_size_-1 < length_)
      length_ = max_size_-1;
    
    str_[length_] = '\0';
    clear_reference();
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
    //std::cout<<getReferCnt()<<std::endl;
    derefer();
    if (COPY_ON_WRITE)
    {
      p_ = str.p_;
      
      str_ = str.str_;
      length_ = str.length_;
      is_attached_ = str.is_attached_;
      max_size_ = str.max_size_;
      refer();
      
      assert(length_<=max_size_);
    }
    else
    {
      p_ = (char*)hlmalloc(get_total_size(str.length()));
      str_ = (CharT*)(p_ + sizeof (ReferT));
      length_ = str.length_;
      max_size_ = length_+1;
      memcpy(str_, str.str_, str.size());
      str_[length_] = '\0';

      is_attached_ = false;
      clear_reference();
    }

    return *this;
  }
  
  SelfT& assign ( const SelfT& str, size_t pos, size_t n )
  {
    if (n != npos)
      assert(str.length_ >= pos+n);

    if (n == npos)
        length_ = str.length_ - pos;
      else
        length_ = n;

    derefer();
    if (COPY_ON_WRITE)
    {
      p_ = str.p_;
      refer();
      
      str_ = str.str_ + pos;
      max_size_ = length_;
      is_attached_ = str.is_attached_;
    }
    else
    {
      p_ = (char*)hlmalloc(get_total_size(length_));
      str_ = (CharT*)(p_+sizeof (ReferT));
      max_size_ = length_+1;
      
      for(size_t i=0; i<length_; i++)
        str_[i] = str[i+pos];

      str_[length_] = '\0';
      is_attached_ = false;
      clear_reference();
    }

    return *this;
  }
  
  SelfT& assign ( const CharT* s, size_t n )
  {
    derefer();
    
    p_ = (char*)hlmalloc(get_total_size(n));
    str_ = (CharT*)(p_+sizeof (ReferT));

    for(size_t i=0; i<n; i++)
      str_[i] = s[i];

    length_ = n;

    is_attached_ = false;
    max_size_ = length_ +1 ;
    str_[length_] = '\0';

    clear_reference();

    return *this;
  }

  SelfT& assign ( const CharT* s )
  {
    assert (s !=NULL);

    //std::cout<<"--"<<getReferCnt()<<std::endl;
    derefer();
    //std::cout<<"++"<<getReferCnt()<<std::endl;
    
    length_ = getLen(s);

    p_ = (char*)hlmalloc(get_total_size(length_));
    str_ = (CharT*)(p_+sizeof (ReferT));
    
    for(size_t i=0; i<length_; i++)
      str_[i] = s[i];

    is_attached_ = false;
    max_size_ = length_ + 1;
    str_[length_] = '\0';
    clear_reference();

    return *this;
  }

  SelfT& assign ( size_t n, CharT c )
  {
    derefer();
    
    length_ = n;
    p_ = (char*)hlmalloc(get_total_size(length_));
    str_ = (CharT*)(p_+sizeof (ReferT));
    
    for(size_t i=0; i<length_; i++)
      str_[i] = c;

    is_attached_ = false;
    max_size_ = length_+1;
    str_[length_] = '\0';
    clear_reference();
    
    return *this;
  }

  SelfT& assign (const std::vector<CharT>& v)
  {
    derefer();

    length_ = v.size();
    p_ = (char*)hlmalloc(get_total_size(length_));
    str_ = (CharT*)(p_+sizeof (ReferT));
    
    for(size_t i=0; i<length_; i++)
      str_[i] = v[i];

    is_attached_ = false;
    max_size_ = length_+1;
    str_[length_] = '\0';
    clear_reference();

    return *this;
  }

  SelfT& assign (const std::string& str)
  {
    derefer();

    CharT* s = (CharT*)str.c_str();
    
    length_ = str.length()/sizeof(CharT);
    p_ = (char*)hlmalloc(get_total_size(length_));
    str_ = (CharT*)(p_+sizeof (ReferT));
    
    for(size_t i=0; i<length_; i++)
      str_[i] = s[i];

    is_attached_ = false;
    max_size_ = length_+1;
    str_[length_] = '\0';
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
const size_t vector_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::npos = -1;


template <
  class CHAR_TYPE ,int COPY_ON_WRITE, uint8_t APPEND_RATE
  >
const float vector_string<CHAR_TYPE ,COPY_ON_WRITE, APPEND_RATE>::append_rate_ = (float)APPEND_RATE/100.+1.;


NS_IZENELIB_AM_END
#endif

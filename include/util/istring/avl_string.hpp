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
class avl_string
{  
public:
  typedef CHAR_TYPE value_type;
  typedef CHAR_TYPE CharT;
  typedef uint16_t  ReferT;
  typedef vector_string<CHAR_TYPE, COPY_ON_WRITE, IS_BALANCED, BUCKET_BYTES> SelfT;
  typedef std::size_t size_t;

  enum 
    {
      BUCKET_LENGTH = BUCKET_BYTES/sizeof(CharT)
    };

  
  static const size_t npos;
protected:

  struct LEEF_
  {
    char bucket_[BUCKET_BYTES+sizeof(ReferT)+sizeof(LEEF_*)];

    inline LEEF_()
    {
      *(ReferT*)(bucket_ + BUCKET_BYTES) = 1;
      *(LEEF_*)(bucket_ + BUCKET_BYTES + sizeof(ReferT)) = NULL;
    }
    
    inline CharT& operator [] (size_t i)
    {
      assert(i<BUCKET_LENGTH);
      return *((CharT*)bucket_ + i);
    }

    inline size_t length()const
    {
      size_t i=0;
      for (;i<BUCKET_LENGTH; i++)
        if (*((CharT*)bucket_ + i)=='\0')
          return i;

      assert(false);
    }

    inline void refer()
    {
      *(ReferT*)(bucket_ + BUCKET_BYTES) += 1;
    }

    inline bool is_refered()
    {
      if (*(ReferT*)(bucket_ + BUCKET_BYTES)>1)
        return true;
      return false;
    }

    inline void derefer()
    {
      if (*(ReferT*)(bucket_ + BUCKET_BYTES) != 0)
        *(ReferT*)(bucket_ + BUCKET_BYTES) -= 1;
    }

    LEEF_* next()
    {
      return *(LEEF_**)(bucket_ + BUCKET_BYTES + sizeof(ReferT));
    }

    LEEF_** next_addr()
    {
      return (LEEF_**)(bucket_ + BUCKET_BYTES + sizeof(ReferT));
    }
    
    void set_next(LEEF_* lf)
    {
      *(LEEF**)(bucket_ + BUCKET_BYTES + sizeof(ReferT)) = lf;
    }
    
    void clear_reference()
    {
      *(ReferT*)(bucket_ + BUCKET_BYTES) = 1;
    }    
    
  }
    ;
    
  struct NODE_
  {
    size_t left_len_;
    uint8_t ldepth_;
    uint8_t rdepth__;
    uint64_t lptr_;
    uint64_t rptr_;
    NODE_* pptr_;

    inline NODE_()
    {
      left_len_ = 0;
      ldepth_ = rdepth_ = 0;
      lptr_ = rptr_ = 0;
      pptr_ = 0;
    }
    
    inline void set_right(NODE_* n)
    {
      rptr_ = (uint64_t)n;
    }

    inline void set_right(LEEF_* n)
    {
      rptr_ = (uint64_t)n;
      is_bottom_ = true;
    }

    inline void set_left(NODE_* n)
    {
      rptr_ = (uint64_t)n;
    }

    inline void set_left(LEEF_* n)
    {
      rptr_ = (uint64_t)n;
      is_bottom_ = true;
    }

    inline NODE_* get_right_node()
    {
      return (NODE_*)rptr_;
    }

    inline NODE_* get_left_node()
    {
      return (NODE_*)lptr_;
    }

    inline LEEF_* get_right_leef()
    {
      return (LEEF_*)rptr_;
    }

    inline LEEF_* get_left_leef()
    {
      return (LEEF_*)lptr_;
    }

    inline void inc_ldepth()
    {
      ldepth_++;
    }

    inline void inc_rdepth()
    {
      rdepth_++;
    }

    inline void dec_ldepth()
    {
      if (ldepth_!=0)
        ldepth_--;
    }

    inline void dec_rdepth()
    {
      if (rdepth_!=0)
        rdepth_--;
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
  NODE_* root_;
  LEEF_* head_;
  size_t length_;

  inline bool refer()
  {
    if (!COPY_ON_WRITE)
      return false;

    //lock
    LEEF_* lf = head_;
    while (lf !=NULL)
    {
      lf->refer();
      lf = lf->next();
    }

    return true;
  }

  void derefer(NODE_* n)
  {
    if (n == NULL)
      return;

    if (n->ldepth_>1 && n->get_left_node()!=NULL)
      derefer(n->get_left_node());

    if (n->rdepth_>1 && n->get_right_node()!=NULL)
      derefer(n->get_right_node());

    hlfree(n);
  }

  void derefer(LEEF_* n)
  {
    if (n == NULL)
      return;

    if (!COPY_ON_WRITE)
    {
      derefer(n->next());
      hlfree(n);
      return;
    }

    //lock
    derefer(n->next());

    n->derefer();
    if (n->is_refered())
      return;
    
    hlfree(n);
  }

  void derefer()
  {
    derefer(root_);
    derefer(head_);
    return ;    
  }

  void clear_reference(LEEF_* lf)
  {
    if (lf == NULL)
      return;

    clear_reference(lf->next());
    lf->clear_reference();
  }
  
  void clear_reference()
  {     
    if (!COPY_ON_WRITE)
      return ;

    //lock
    clear_reference(head_);
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

  void string_up(const NODE_* n, LEEF_** last_lf)
  {
    if (n == NULL)
      return;
    
    if (n->ldepth_==1)
    {
      *last_lf = (LEEF_*)n->lptr_;
      last_lf = ((LEEF_*)n->lptr_)->next_addr();
    }
    else
      string_up((NODE_*)n->lptr_, last_lf);

    if (n->rdepth_ ==1)
    {
      *last_lf = (LEEF_*)n->rptr_;
      last_lf = ((LEEF_*)n->rptr_)->next_addr();
    }
    else
      string_up((NODE_*)n->rptr_, last_lf);
    
  }
  
  void duplicate(NODE_** n1/*new one*/, const NODE_& n2)
  {
    *n1 = (NODE_*)hlmalloc(sizeof(NODE_));
    *n1 = n2;

    if (n2.is_bottom_)
    {
      if (COPY_ON_WRITE)
      {
        if (n2.get_right_leef()!=NULL)
          n2.get_right_leef()->refer();
        if (n2.get_left_leef()!=NULL)
          n2.get_left_leef()->refer();
        
        return;
      }
      
      if (n2.get_left_leef()!= NULL)
      {
        (*n1)->set_left((LEEF_*)hlmalloc(sizeof(LEEF_)));
        (*n1)->get_left_leef()->clear_reference();
        (*n1)->get_left_leef()->set_next(NULL);
      }
      
      if (n2.get_right_leef()!= NULL)
      {
        (*n1)->set_right((LEEF_*)hlmalloc(sizeof(LEEF_)));
        (*n1)->get_right_leef()->clear_reference();
        (*n1)->get_right_leef()->set_next(NULL);
      }
    }

    if ((NODE_*)n2.lptr_!=NULL)
      duplicate(&((NODE_*)(*n1)->lptr_), *(NODE_*)n2.lptr_);
    if ((NODE_*)n2.rptr_!=NULL)
      duplicate(&((NODE_*)(*n1)->rptr_), *(NODE_*)n2.rptr_);
  }
  
  void duplicate_self(NODE_** root, LEEF_** head)
  {
    duplicate(root, root_);
    if (!COPY_ON_WRITE)
      string_up(*root, head);
    else
      *head = head_;
  }
  
public:
  explicit avl_string()
  {
    length_ = 0;
    root_ = (NODE_*)hlmalloc(sizeof(NODE_));
    *root_ = NODE_();
    root_->add_rdepth();
    root_->add_ldepth();
    LEEF_* lf = (LEEF_*)hlmalloc(sizeof(LEEF_));
    *lf = LEEF_();
    root_->set_right(lf);
  }

  vector_string ( const SelfT& str )
  //    :p_(NULL), is_attached_(false),str_(NULL),length_(0),max_size_(0)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(str);
  }
  
  vector_string ( const SelfT& str, size_t pos, size_t n = npos )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(str, pos, n);
  }
  
  vector_string ( const CharT * s, size_t n )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(s, n);
  }
  
  vector_string ( const CharT * s )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(s);
  }
  
  vector_string ( size_t n, CharT c )
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(n, c);
  }

  vector_string (const std::vector<CharT>& v)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(v);
  }

  vector_string (const std::string& str)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(str);
  }
  

  template<class InputIterator> vector_string (InputIterator begin, InputIterator end)
  {
    length_ = 0;
    root_ =NULL;
    head_ = NULL;
    assign(begin(), end);
  }

  virtual ~vector_string()
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
    derefer();

    str.duplicate_self(&root_, &head_);
  
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

    avl_string();
    
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

NS_IZENELIB_AM_END
#endif

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
  typedef uint32_t  ReferT;
  typedef bbt_string<CHAR_TYPE, COPY_ON_WRITE, BUCKET_BYTES> SelfT;
  typedef std::size_t size_t;

  enum 
    {
      BUCKET_LENGTH = BUCKET_BYTES/sizeof(CharT) -1,
      BUCKET_SIZE = BUCKET_BYTES + sizeof(ReferT)
    };

  
  static const size_t npos;
protected:

  struct BUCKET_
  {
    char p_[BUCKET_SIZE];

    inline BUCKET_()
    {
      *(ReferT*)(p_ + BUCKET_BYTES) = 1;
    }
    
    inline CharT& operator [] (size_t i)
    {
      assert(i<=BUCKET_LENGTH);
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
    
    void clear_reference()
    {
      *(ReferT*)(p_ + BUCKET_BYTES) = 1;
    }    

  friend std::ostream& operator << (std::ostream& os, const BUCKET_& b)
    {
      os<<b.p_<<"("<<*(ReferT*)(b.p_ + BUCKET_BYTES)<<")";
      return os;
    }
    
  }
    ;

  struct INDEX_
  {
    size_t len_;
    BUCKET_* bptr_;

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
    
  friend std::ostream& operator << (std::ostream& os, const INDEX_& b)
    {
      os<<"["<<b.len_<<"]"<<*b.bptr_;
      return os;
    }
    
  };
  
public:
  class const_iterator;
  
  class iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {

  friend class const_iterator;
  public:
    iterator(INDEX_* indice=NULL, size_t ii=0 , size_t bi=0 )
    {
      indice_ = indice;
      ii_ = ii;
      bi_ = bi;
    }
    
    ~iterator() {}

    // The assignment and relational operators are straightforward
    iterator& operator = (const iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;
      
      return(*this);
    }

    bool operator==(const iterator& other)const
    {
      return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
    }

    bool operator!=(const iterator& other)const
    {
      return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
    }

    bool operator < (const iterator& other)const
    {
      return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
    }
    
    bool operator > (const iterator& other)const
    {
      return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    iterator& operator++()
    {
      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(*this);
      }

      bi_++;
      if (indice_[ii_].bptr_->p_[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return *this;
      
    }

    iterator operator++(int)
    {
      iterator tmp(*this);
      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(tmp);
      }

      bi_++;
      if (indice_[ii_].bptr_->p_[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    iterator& operator--()
    {
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_-lastl - 1;
      }
      else
        bi_--;

      return(*this);
    }

    iterator operator--(int)
    {
      iterator tmp(*this);
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = (ii_==0? 0: indice_[ii_-1].len_);
        bi_ = indice_[ii_].len_- lastl - 1;
      }
      else
        bi_--;

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    CharT& operator*()
    {
      assert(indice_[ii_].bptr_!=NULL);
      
      if (indice_[ii_].bptr_->is_refered())
      {
        BUCKET_* bu = SelfT::new_bucket();
        memcpy(bu, indice_[ii_].bptr_, BUCKET_SIZE);
        indice_[ii_].bptr_->derefer();
        indice_[ii_].bptr_ = bu;
        bu->clear_reference();
      }

      return indice_[ii_].bptr_->p_[bi_];
    }
    
    iterator operator + (size_t i)const
    {
      iterator tmp (*this);

      for (;i>0; i--)
        ++tmp;
      
      return tmp;
    }

    iterator operator - (size_t i)const
    {
      iterator tmp(*this);

      for (;i>0; i--)
        --tmp;
      
      return tmp;
    }

    size_t operator - (const iterator& other) const
    {
      iterator tmp(other);
      size_t i = 0;

      if (indice_ != other.indice_)
        return -1;

      if (*this>other)
        for (; tmp!=*this; i++)
          ++tmp;
      
      if (*this<other)
        for (; tmp!=*this; i--)
          --tmp;      
      
      return i;
    }
    
  protected:
    size_t bi_;//index in bucket
    size_t ii_;// indice index
    INDEX_* indice_;
  };

  
  class const_iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {
  public:
    const_iterator(const INDEX_* indice=NULL, size_t ii=0 , size_t bi=0 )
    {
      indice_ = indice;
      ii_ = ii;
      bi_ = bi;
    }

    const_iterator(const iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;
    }
    
    ~const_iterator() {}

    // The assignment and relational operators are straightforward
    const_iterator& operator=(const const_iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;
      
      return(*this);
    }
    
    // The assignment and relational operators are straightforward
    const_iterator& operator = (const iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;

      return(*this);
    }

    bool operator==(const const_iterator& other) const
    {
      return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
    }

    bool operator < (const const_iterator& other) const
    {
      return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
    }
    
    bool operator > (const const_iterator& other) const
    {
      return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
    }

    bool operator!=(const const_iterator& other) const
    {
      return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator++()
    {
      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(*this);
      }

      bi_++;
      if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp(*this);

      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(tmp);
      }

      bi_++;
      if ((*indice_[ii_].bptr_)[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }      

      return(tmp);
    }
    
    // Update my state such that I refer to the next element in the
    // SQueue.
    const_iterator& operator--()
    {
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
      }
      else
        bi_--;

      return(*this);
    }

    const_iterator operator--(int)
    {
      iterator tmp(*this);
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
      }
      else
        bi_--;

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const CharT& operator*() const
    {
      assert(indice_[ii_].bptr_ != NULL);
      
      return (*indice_[ii_].bptr_)[bi_];
    }
    
    uint64_t operator - (const const_iterator& other) const
    {
      const_iterator tmp(other);
      size_t i = 0;

      if (indice_ != other.indice_)
        return -1;

      if (*this>other)
        for (; tmp!=*this; i++)
          ++tmp;
      
      if (*this<other)
        for (; tmp!=*this; i--)
          --tmp;      
      
      return i;
    }
    
    const_iterator operator + (size_t i)const
    {
      const_iterator tmp(*this);

      for (;i>0; i--)
        ++tmp;
      
      return tmp;
    }

    const_iterator operator - (size_t i)const
    {
      const_iterator tmp(*this);

      for (;i>0; i--)
        --tmp;
      
      return tmp;
    }

  private:
    size_t bi_;//index in bucket
    size_t ii_;// indice index
    const INDEX_* indice_;
  };


  class const_reverse_iterator;

  class reverse_iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {
    
  friend class const_reverse_iterator;
  public:
    reverse_iterator(INDEX_* indice, size_t ii=0 , size_t bi=0 )      
    {
      indice_ = indice;
      ii_ = ii;
      bi_ = bi;
    }


    ~reverse_iterator() {}
    
    // The assignment and relational operators are straightforward
    reverse_iterator& operator=(const reverse_iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;
      
      return(*this);
    }

    bool operator==(const reverse_iterator& other)const
    {
      return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
    }
    
    bool operator!=(const reverse_iterator& other)const
    {
      return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
    }
    
    bool operator > (const reverse_iterator& other)const
    {
      return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
    }
    
    bool operator < (const reverse_iterator& other)const
    {
      return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    reverse_iterator& operator++()
    {
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
      }
      else
        bi_--;

      return(*this);
    }

    reverse_iterator operator++(int)
    {
      reverse_iterator tmp(*this);
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_-lastl - 1;
      }
      else
        bi_--;

      return(tmp);
    }
    
    // Update my state such that I refer to the next element in the
    // SQueue.
    reverse_iterator& operator--()
    {
      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(*this);
      }

      bi_++;
      if (indice_[ii_].bptr_->p_[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return(*this);
    }

    reverse_iterator operator--(int)
    {
      reverse_iterator tmp(*this);

      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(tmp);
      }

      bi_++;
      if (indice_[ii_].bptr_->p_[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    CharT& operator*()
    {
      assert(indice_[ii_].bptr_ != NULL);
      
      if (indice_[ii_].bptr_->is_refered())
      {
        BUCKET_* bu = new_bucket();
        memcpy(bu, indice_[ii_].bptr_, BUCKET_SIZE);
        indice_[ii_].bptr_->derefer();
        indice_[ii_].bptr_ = bu;
        bu->clear_reference();
      }

      return indice_[ii_].bptr_->p_[bi_];
    }
    
    reverse_iterator operator + (size_t i)const
    {
      reverse_iterator tmp(*this);

      for (;i>0; i--)
        --tmp;

      return *this;
    }

    reverse_iterator operator - (size_t i)const
    {
      reverse_iterator tmp(*this);

      for (;i>0; i--)
        ++tmp;
      
      return tmp;
    }

    size_t operator - (const reverse_iterator& other) const
    {
      reverse_iterator tmp(other);
      size_t i = 0;

      if (indice_ != other.indice_)
        return -1;

      if (*this>other)
        for (; tmp!=*this; i++)
          ++tmp;
      
      if (*this<other)
        for (; tmp!=*this; i--)
          --tmp;      
      
      return i;
    }

  private:
    size_t bi_;//index in bucket
    size_t ii_;// indice index
    INDEX_* indice_;
  };

  
  class const_reverse_iterator :
    public std::iterator<std::forward_iterator_tag, CharT> {
  public:
    const_reverse_iterator(const INDEX_* indice, size_t ii=0 , size_t bi=0 )
    {
      indice_ = indice;
      ii_ = ii;
      bi_ = bi;
    }


    const_reverse_iterator(const reverse_iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;
    }
    
    ~const_reverse_iterator() {}

    // The assignment and relational operators are straightforward
    const_reverse_iterator& operator=(const const_reverse_iterator& other)
    {
      ii_ = other.ii_;
      bi_ = other.bi_;
      indice_ = other.indice_;
      return(*this);
    }

    bool operator==(const const_reverse_iterator& other) const
    {
      return(indice_ == other.indice_ && ii_ == other.ii_ && bi_ == other.bi_ );
    }

    bool operator!=(const const_reverse_iterator& other) const
    {
      return(indice_ != other.indice_ || ii_ != other.ii_ || bi_ != other.bi_ );
    }
    
    bool operator > (const const_reverse_iterator& other)const
    {
      return(indice_ == other.indice_ && (ii_ < other.ii_ || ii_==(size_t)-1 && other.ii_!=(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ < other.bi_ );
    }

    bool operator < (const const_reverse_iterator& other)const
    {
      return(indice_ == other.indice_ && (ii_ > other.ii_ || ii_!=(size_t)-1 && other.ii_==(size_t)-1)
        || indice_ == other.indice_ && ii_ == other.ii_ && bi_ > other.bi_ );
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_reverse_iterator& operator++()
    {
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_-indice_[ii_-1].len_ - 1;
      }
      else
        bi_--;

      return(*this);
    }

    const_reverse_iterator operator++(int)
    {
      const_reverse_iterator tmp(*this);
      if (indice_ == NULL)
        return(*this);
      
      if (bi_==0)
      {
        if (ii_!=(size_t)-1)
          ii_--;
        
        if (ii_==(size_t)-1)
          return(*this);
        
        size_t  lastl = ii_==0? 0: indice_[ii_-1].len_;
        bi_ = indice_[ii_].len_ - lastl - 1;
      }
      else
        bi_--;

      return(tmp);
    }

    // Update my state such that I refer to the next element in the
    // SQueue.
    const_reverse_iterator& operator--()
    {
      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(*this);
      }

      bi_++;
      if (indice_[ii_].bptr_->p_[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return(*this);
    }

    const_reverse_iterator operator--(int)
    {
      const_reverse_iterator tmp(*this);
      if (indice_ == NULL)
        return(*this);

      if (ii_==(size_t)-1)
      {
        ii_ = 0;
        bi_ = 0;
        return(tmp);
      }

      bi_++;
      if (indice_[ii_].bptr_->p_[bi_]=='\0' || bi_>=BUCKET_LENGTH)
      {
        bi_ = 0;
        ii_++;
      }

      return(tmp);
    }

    // Return a reference to the value in the node.  I do this instead
    // of returning by value so a caller can update the value in the
    // node directly.
    const CharT& operator*() const
    {
      return indice_[ii_].bptr_->p_[bi_];
    }

    
    const_reverse_iterator operator + (size_t i)const
    {
      const_reverse_iterator tmp(*this);

      for (;i>0; i--)
        --tmp;

      return *this;
    }

    const_reverse_iterator
    operator - (size_t i)const
    {
      const_reverse_iterator tmp(*this);

      for (;i>0; i--)
        ++tmp;
      
      return tmp;
    }

    size_t operator - (const const_reverse_iterator& other) const
    {
      const_reverse_iterator tmp(other);
      size_t i = 0;

      if (indice_ != other.indice_)
        return -1;

      if (*this>other)
        for (; tmp!=*this; i++)
          ++tmp;
      
      if (*this<other)
        for (; tmp!=*this; i--)
          --tmp;      
      
      return i;
    }

  private:
    size_t bi_;//index in bucket
    size_t ii_;// indice index
    const INDEX_* indice_;
  };

  
protected:
  INDEX_* indice_;
  size_t idx_len_;
  size_t idx_size_;
  size_t length_;

  inline bool refer_all()
  {
    if (!COPY_ON_WRITE)
      return false;

    //lock
    for (size_t i=0; i<idx_len_; i++)
    {
      indice_[i].bptr_->refer();
    }
    
    return true;
  }

  inline bool refer(size_t i)
  {
    if (!COPY_ON_WRITE)
      return false;

    //lock
    assert(i<idx_len_);
    indice_[i].bptr_->refer();
    
    return true;
  }

  void derefer_all()
  {
    for (size_t i=0; i<idx_size_; i++)
      derefer(i);
        
    idx_len_ = length_ = 0;
  }

  void derefer(size_t i)
  {
    //lock
    assert(i<idx_size_);
    
    if (!COPY_ON_WRITE)
    {
      if (indice_[i].bptr_!=NULL)
        hlfree(indice_[i].bptr_);
      indice_[i].bptr_  = NULL;
      indice_[i].len_ = 0;
      return;
    }

    if (indice_[i].bptr_ == NULL)
      return;
    
    if (!indice_[i].bptr_->is_refered())
      hlfree(indice_[i].bptr_);
    else
      indice_[i].bptr_->derefer();
    
    indice_[i].bptr_ = NULL;
  }
  
  void clear_all_reference()
  {     
    if (!COPY_ON_WRITE)
      return ;

    
    //lock
    for (size_t i=0; i<idx_len_; i++)
    {
      indice_[i].bptr_->clear_reference();
    }

  }
  
  void clear_reference(size_t i)
  {     
    if (!COPY_ON_WRITE)
      return ;

    assert(i<idx_size_);
    
    //lock
    indice_[i].bptr_->clear_reference();

  }

  inline void clear_indice(size_t i, size_t n)
  {
    for (size_t j=0;j<n;j++)
    {
      indice_[i+j].bptr_ = NULL;
      indice_[i+j].len_ = 0;
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

  static inline BUCKET_* new_bucket()
  {
    BUCKET_* bu = (BUCKET_*)hlmalloc(BUCKET_SIZE);
    bu->clear_reference();
    return bu;
  }

  inline BUCKET_* new_bucket(size_t i)const
  {
    assert(i<idx_len_ && indice_[i].bptr_!=NULL);
    
    BUCKET_* bu = new_bucket();
    memcpy(bu, indice_[i].bptr_, BUCKET_SIZE);
    bu->clear_reference();
    
    return bu;
  }
  
  void duplicate(size_t i)
  {  
    BUCKET_* bu = new_bucket();
    memcpy(bu, indice_[i].bptr_, BUCKET_SIZE);
    bu->clear_reference();

    if (COPY_ON_WRITE)
      derefer(i);
    
    indice_[i].bptr_ = bu;
  }
  
  size_t duplicate(INDEX_** indice) const
  {
    *indice = (INDEX_*)hlmalloc(idx_len_*sizeof(INDEX_));
    memcpy (*indice, indice_, idx_len_*sizeof(INDEX_));

    for (size_t i=0; i<idx_len_; i++)
    {
      if (COPY_ON_WRITE)
      {
        (*indice)[i].bptr_->refer();
        continue;
      }

      BUCKET_* bu = new_bucket();
      memcpy(bu, (*indice)[i].bptr_, BUCKET_SIZE);
      (*indice)[i].bptr_ = bu;
    }


    return idx_len_;
  }
  
  size_t duplicate(size_t start, size_t n, INDEX_** indice) const
  {
    assert(start + n <= length_);
    
    size_t s = binary_search(start, 0, idx_len_-1);
    // std::cout<<start<<" "<<idx_len_<<" "<<s<<std::endl;
//     display();
    
    size_t e = binary_search(start+n-1, s, idx_len_-1);
    
    size_t last_s = s==0? 0: indice_[s-1].len_;
    *indice = (INDEX_*)hlmalloc((e-s+1)*sizeof(INDEX_));

    // if entire bucket
    if (start-last_s == 0 && COPY_ON_WRITE)
    {
      (*indice)[0].bptr_ = indice_[s].bptr_;
      if (n >= indice_[s].len_-start)
        (*indice)[0].bptr_->refer();
      else
        (*indice)[0].bptr_ = new_bucket(s);
    }
    else
    {
      (*indice)[0].bptr_ = new_bucket();
      memcpy((*indice)[0].bptr_->p_, indice_[s].bptr_->p_ + (start-last_s),
             BUCKET_SIZE-(start-last_s)*sizeof(CharT));
      (*indice)[0].bptr_->clear_reference();
    }

    (*indice)[0].len_ = n>indice_[s].len_-start ? indice_[s].len_-start:n;
    (*(*indice)[0].bptr_)[(*indice)[0].len_] = '\0';
    last_s = indice_[s].len_;
    s++;
    size_t i=1;

    while (s<=e)
    {
      if (COPY_ON_WRITE)
      {
        (*indice)[i].bptr_ =  indice_[s].bptr_;
        if (n-(*indice)[i-1].len_ >= indice_[s].len_-last_s)
          (*indice)[i].bptr_->refer();
        else
          (*indice)[i].bptr_ = new_bucket(s);
      }
      else
      {
        (*indice)[i].bptr_ = new_bucket();
        memcpy((*indice)[i].bptr_->p_, indice_[s].bptr_->p_,
               (indice_[s].len_-last_s)*sizeof(CharT));
        (*indice)[i].bptr_->clear_reference();
      }

      (*indice)[i].len_ = (*indice)[i-1].len_ +
        (n-(*indice)[i-1].len_ > indice_[s].len_-last_s ? indice_[s].len_-last_s:n-(*indice)[i-1].len_);
      (*indice)[i].bptr_->p_[(*indice)[i].len_-(*indice)[i-1].len_] = '\0';
      last_s = indice_[s].len_;
      s++;
      i++;
      
    }
    
    return i;
  }
    
  size_t binary_search(size_t i, size_t start, size_t end)const
  {
    if (end == start)
      return end;

    assert(start<end);
    
    size_t mid = (start+end)/2;
    if (i+1 > indice_[mid].len_)
      return binary_search(i, mid+1, end);
    else if (mid==0 || i+1 <= indice_[mid].len_ && i+1 > indice_[mid-1].len_  )
      return mid;
    else if (i+1 < indice_[mid].len_)
      return binary_search(i, start, mid-1);

    return -1;
  }

  bool is_refered(size_t i)
  {
    //lock
    assert(i<idx_size_);
    
    if (!COPY_ON_WRITE)
      return false;

    if (indice_[i].bptr_==NULL)
      return false;
    
    return indice_[i].bptr_->is_refered();
    
  }
  
public:
  explicit bbt_string()
  {
    idx_len_ = length_ = 0;
    indice_ = (INDEX_*)hlmalloc(BUCKET_LENGTH*sizeof(INDEX_));
    clear_indice(0, BUCKET_LENGTH);
    
    idx_size_ = BUCKET_LENGTH;
  }

  bbt_string ( const SelfT& str )
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(str);
  }
  
  bbt_string ( const SelfT& str, size_t pos, size_t n = npos )
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    assign(str, pos, n);
  }
  
  bbt_string ( const CharT * s, size_t n )
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(s, n);
  }
  
  bbt_string ( const CharT * s )
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(s);
  }
  
  bbt_string ( size_t n, CharT c )
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(n, c);
  }

  bbt_string (const std::vector<CharT>& v)
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(v);
  }

  bbt_string (const std::string& str)
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(str);
  }
  

  template<class InputIterator> bbt_string (InputIterator begin, InputIterator end)
  {
    length_ = idx_len_ = idx_size_ = 0;
    indice_ =NULL;
    
    assign(begin(), end);
  }

  virtual ~bbt_string()
  {
    clear();
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
    return const_iterator(indice_, 0, 0);
  }

  iterator begin()
  {
    return iterator(indice_, 0, 0);
  }

  const_iterator end() const
  {
    if (length_==0)
      return begin();
    
    //return const_iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_));
    return const_iterator(indice_, idx_len_, 0);
  }

  iterator end()
  {
    if (length_==0)
      return begin();

    //return iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_));
    return iterator(indice_, idx_len_, 0);
  }

  reverse_iterator rbegin()
  {
    return reverse_iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_)-1);
  }

  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(indice_, idx_len_-1, length_-(idx_len_-1==0? 0: indice_[idx_len_-2].len_)-1);
  }

  reverse_iterator rend()
  {
    if (length_==0)
      return rbegin();
    
    return reverse_iterator(indice_, -1, 0);
  }

  const_reverse_iterator rend() const
  {
    if (length_==0)
      return rbegin();
    
    return const_reverse_iterator(indice_, -1, 0);
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

  size_t indice_length() const
  {
    return idx_len_;
  }

  size_t max_size ( ) const
  {
    return idx_len_ * BUCKET_LENGTH;
  }

  void resize (size_t n, CharT c)
  {
    if (n == length_ )
      return;
    
    if (n < length_)
    {
      size_t i = binary_search(n-1, 0, idx_len_-1);
      size_t il = i==0? 0: indice_[i-1].len_;
      indice_[i].len_ = n;

      duplicate(i);
      
      (*indice_[i].bptr_)[n-il]= '\0';
      
      for (size_t j=i+1; j<idx_size_; j++)
        derefer(j);
      
      idx_len_ = i+1;
      length_ = n;
      
      return;
    }

    append(n-length_, c);
  }

  void resize ( size_t n )
  {
    if (n == max_size())
      return;

    resize(n, 0);
  }

  size_t capacity ( ) const
  {
    size_t i=0;
    //std::cout<<idx_len_<<"-"<<idx_size_<<std::endl;
    
    for (; i<idx_size_; i++)
      if (indice_[i].bptr_ == NULL)
        break;
    
    return i * sizeof(CharT) * BUCKET_LENGTH;
  }

  void reserve ( size_t res_arg=0 )
  {
    if (res_arg<= capacity())
      return;
    
    size_t n = res_arg/sizeof(CharT);
    n = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0?0:1)-idx_len_;

    assert(n>0);

    if (idx_len_+n > idx_size_){
      indice_ = (INDEX_*)hlrealloc(indice_, (idx_len_+n)*sizeof(INDEX_));
      clear_indice(idx_size_, idx_len_+n-idx_size_);
      idx_size_ = idx_len_+n;
    }

    for (size_t i=0; i<n; i++)
    {
      if (indice_[idx_len_+i].bptr_!=NULL)
      {
        if (is_refered(idx_len_+i))
          derefer(idx_len_+i);
      }
      

      if (indice_[idx_len_+i].bptr_ == NULL)
        indice_[idx_len_+i].bptr_ = new_bucket();
      
      indice_[idx_len_+i].len_ = indice_[idx_len_+i-1].len_;
    }

    //idx_len_ += n;
  }

  void clear()
  {
    derefer_all();
    length_ = idx_len_ = idx_size_ = 0;

    if (indice_!=NULL)
      hlfree(indice_);
    indice_ = NULL;
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

    size_t t = binary_search(pos, 0, idx_len_-1);
    size_t l = t==0? 0: indice_[t-1].len_;

    return indice_[t].bptr_->p_[pos-l];
  }
  
  CharT& operator[] ( size_t pos )
  {
    assert(pos<length_);
    
    size_t t = binary_search(pos, 0, idx_len_-1);
    if (is_refered(t))
      duplicate(t);

    size_t l = t==0? 0: indice_[t-1].len_;

    assert(pos>=l);

    return indice_[t].bptr_->p_[pos-l];
    
  }

  const CharT& at ( size_t pos ) const
  {
    assert(pos<length_);

    size_t t = binary_search(pos, 0, idx_len_-1);
    size_t l = t==0? 0: indice_[t-1].len_;

    return indice_[t].bptr_->p_[pos-l];
  }
  
  CharT& at ( size_t pos )
  {
    assert(pos<length_);
    size_t t = binary_search(pos, 0, idx_len_-1);
    if (is_refered(t))
      duplicate(t);

    size_t l = t==0? 0: indice_[t-1].len_;

    return indice_[t].bptr_->p_[pos-l];
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
    if (str.length()==0)
      return *this;

//     if (idx_len_>0)
//     {
//       size_t ei = indice_[idx_len_-1].len_- (idx_len_>1? indice_[idx_len_-2].len_: 0);
//       if (ei+str.length() <= BUCKET_LENGTH)
//       {
    //if (is_refered(idx_len_-1))
    //    duplicate(idx_len_-1);
//         for (const_iterator i=str.begin(); i!= str.end();i++, ei++, length_++)
//           (*indice_[idx_len_-1].bptr_)[ei] = *i;
//         (*indice_[idx_len_-1].bptr_)[ei] = '\0';
//         indice_[idx_len_-1].len_ += str.length();
//         return *this;
//       }
//     }
    
    //extend indice
    if (str.indice_length()+idx_len_>idx_size_)
    {
      indice_ = (INDEX_*)hlrealloc(indice_, (str.indice_length()+idx_len_)*sizeof(INDEX_));
      clear_indice(idx_size_, idx_len_+str.indice_length()-idx_size_);
      idx_size_ = str.indice_length()+idx_len_;
    }

    for (size_t j=idx_len_; j<idx_size_;j++)
      derefer(j);
    
    memcpy(indice_+idx_len_, str.indice_, str.indice_length()*sizeof(INDEX_));

    //copy str into
    for (size_t i=idx_len_; i<idx_len_+str.indice_length(); i++)
    {
      if (COPY_ON_WRITE)
      {
        indice_[i].bptr_->refer();
      }
      else
      {
        duplicate(i);
      }

      indice_[i].len_ += length_;
    }

    idx_len_ += str.indice_length();
    length_ += str.length();
    
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

    size_t k=0;
    if (idx_len_>0)
    {
      size_t ei = indice_[idx_len_-1].len_- (idx_len_>1? indice_[idx_len_-2].len_: 0)-1;
      if (ei+1<BUCKET_LENGTH)
      {
        if (is_refered(idx_len_-1))
          duplicate(idx_len_-1);
        
        size_t j=ei+1;
        for (; j<BUCKET_LENGTH && k<n; j++, k++)
          (*indice_[idx_len_-1].bptr_)[j] = s[k];
        indice_[idx_len_-1].len_ += k;
        (*indice_[idx_len_-1].bptr_)[j] = '\0';
        length_ += k;
      }

      if (k == n)
        return *this;
    }
    
    
    size_t ex = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
    //extend indice
    if (ex+idx_len_>idx_size_)
    {
      indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
      clear_indice(idx_size_, ex+idx_len_-idx_size_);
      idx_size_ = ex+idx_len_;
    }
    
    for (size_t j= idx_len_; k<n; j++, idx_len_++)
    {
      if (is_refered(j))
        derefer(j);
      if (indice_[j].bptr_==NULL)
        indice_[j].bptr_ = new_bucket();


      size_t t=0;
      for (; t<BUCKET_LENGTH && k<n; k++,t++, length_++)
        indice_[j].bptr_->p_[t] = s[k];

      indice_[j].bptr_->p_[t] = '\0';
      indice_[j].len_ = indice_[j-1].len_ + t;
    }
    
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

    size_t k=0;
    if (idx_len_>0)
    {
      size_t ei = indice_[idx_len_-1].len_- (idx_len_>1? indice_[idx_len_-2].len_: 0)-1;
      if (ei+1<BUCKET_LENGTH)
      {
        if (is_refered(idx_len_-1))
          duplicate(idx_len_-1);

        size_t j=ei+1;
        for (; j<BUCKET_LENGTH && k<n; j++, k++)
          (*indice_[idx_len_-1].bptr_)[j] = c;
      
        indice_[idx_len_-1].len_ += k;
        (*indice_[idx_len_-1].bptr_)[j] = '\0';
        length_ += k;
      }

      if (k == n)
        return *this;
    }
    
    size_t ex = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
    //extend indice
    if (ex+idx_len_>idx_size_)
    {
      indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
      clear_indice(idx_size_, ex+idx_len_-idx_size_);
      idx_size_ = ex+idx_len_;
    }

    for (size_t j= idx_len_; k<n; j++, idx_len_++)
    {
      if (is_refered(j))
        derefer(j);
      
      if (indice_[j].bptr_ == NULL)
        indice_[j].bptr_ = new_bucket();

      size_t t=0;
      for (;t<BUCKET_LENGTH && k<n; k++,t++,length_++)
        (*indice_[j].bptr_)[t] = c;

      (*indice_[j].bptr_)[t] = '\0';
      indice_[j].len_ = (j>0? indice_[j-1].len_:0) + t;
    }

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

  void push_front(const SelfT& str)
  {
    size_t ex = str.indice_length();
    
    //extend indice
    if (ex+idx_len_>idx_size_)
    {
      indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
      idx_size_ = ex + idx_len_;
    }

    //shift indice
    for (size_t i=idx_len_-1; i!=(size_t)-1; i--)
    {
      indice_[i].len_ += str.length();
      indice_[i+ex] = indice_[i];
    }
    idx_len_ += ex;

    memcpy(indice_, str.indice_, str.indice_length()*sizeof(INDEX_));

    for (size_t i=0; i<str.indice_length(); i++)
    {
      if (COPY_ON_WRITE)
      {
        refer(i);
        continue;
      }

      duplicate(i);
    }

    length_ += str.length();
  }
  
  SelfT& assign ( const SelfT& str )
  {
    clear();

    idx_size_ = idx_len_ = str.duplicate(&indice_);

    length_ = str.length();
      
    return *this;
  }
  
  SelfT& assign ( const SelfT& str, size_t pos, size_t n )
  {
    if (n != npos)
      assert(str.length_ >= pos+n);

    clear();
    
    if (n == npos)
        length_ = str.length_ - pos;
      else
        length_ = n;
    
    idx_len_ = idx_size_ = str.duplicate(pos, length_, &indice_);
    
    return *this;
  }
  
  SelfT& assign ( const CharT* s, size_t n )
  {
    clear();

    idx_len_ = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
    idx_size_ = idx_len_ > BUCKET_LENGTH? idx_len_: BUCKET_LENGTH;
    length_ = n;
    
    indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
    clear_indice(0, idx_size_);

    for (size_t i=0; i<idx_len_; i++)
    {
      indice_[i].bptr_ = new_bucket();
      size_t lastl = i==0?0: indice_[i-1].len_;
      indice_[i].len_ = (n-lastl > BUCKET_LENGTH? BUCKET_LENGTH: n-lastl) + lastl;
      
      memcpy(indice_[i].bptr_->p_, s+lastl, (indice_[i].len_-lastl)*sizeof(CharT));
      (*indice_[i].bptr_)[indice_[i].len_ - lastl] = '\0';
    }
    
    return *this;
  }

  SelfT& assign ( const CharT* s )
  {
    assert (s !=NULL);
    derefer_all();
    
    length_ = getLen(s);

    return assign(s, length_);
  }

  SelfT& assign ( size_t n, CharT c )
  {
    derefer_all();
    
    idx_len_ = n/BUCKET_LENGTH + (n%BUCKET_LENGTH==0? 0: 1);
    idx_size_ = idx_len_ > BUCKET_LENGTH? idx_len_: BUCKET_LENGTH;
    length_ = n;
    
    indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
    memset (indice_, 0, idx_size_*sizeof(INDEX_));

    for (size_t i=0; i<idx_len_; i++)
    {
      indice_[i].bptr_ = new_bucket();
      indice_[i].len_ = n>BUCKET_LENGTH? BUCKET_LENGTH: n;
      n -= indice_[i].len_;

      for (size_t j=0; j<indice_[i].len_; j++)
        (*indice_[i].bptr_)[j] = c;
      
      indice_[i].bptr_->p_[indice_[i].len_] = '\0';
      indice_[i].bptr_->clear_reference();
      if (i>0)
        indice_[i].len_ += indice_[i-1].len_;
    }
    
    return *this;
  }

  SelfT& assign (const std::vector<CharT>& v)
  {
    return assign(v.data(), v.size());
  }

  SelfT& assign (const std::string& str)
  {
    return assign(str.data(), str.length());
  }
  
  template <class InputIterator>
  SelfT& assign ( InputIterator first, InputIterator last )
  {
    derefer_all();
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

    if (this == &str)
    {
      SelfT s = str;
      return insert(pos1, s);
    }

    length_ += str.length();
    size_t s = binary_search(pos1, 0, idx_len_-1);
    size_t last_s = (s==0? 0: indice_[s-1].len_);
    size_t ii = pos1-last_s;

    //within one bucket
    if (str.length() + indice_[s].len_-last_s<=BUCKET_LENGTH)
    {
      //std::cout<<str.length()<<" "<<s<<std::endl;
      //str.display();
      
      if (is_refered(s))
        duplicate(s);
      BUCKET_* bu = indice_[s].bptr_;
      
      for (size_t i = indice_[s].len_-last_s-1; i>ii; i--)
        (*bu)[i+str.length()] = (*bu)[i];
      (*bu)[indice_[s].len_-last_s+str.length()]= '\0';
      
      for (size_t i = 0; i<str.length(); i++)
        (*bu)[ii+1+i] = str[i];

      for (size_t i=s; i<idx_len_; i++)
        indice_[i].len_ += str.length();
      
      return *this;
    }

    size_t ex = str.indice_length();
    BUCKET_* tail = NULL;
    size_t tt = 0;

    //split bucket
    if (ii+1 < indice_[s].len_-last_s)
    {
      ex++;
      //copy head
      if (is_refered(s))
        duplicate(s);

      //copy tail
      tail = new_bucket();
      memcpy(tail, indice_[s].bptr_->p_+ii+1, (BUCKET_LENGTH - (ii+1))*sizeof(CharT));
      tail->clear_reference();
      tt  = indice_[s].len_ - last_s - ii-1;
      (*tail)[tt] = '\0';

      (*indice_[s].bptr_)[ii+1] = '\0';
      indice_[s].len_ = last_s + ii + 1;
    }

    last_s = indice_[s].len_;

    //extend indice
    if (ex + idx_len_>idx_size_)
    {
      indice_ = (INDEX_*)hlrealloc(indice_, (ex+idx_len_)*sizeof(INDEX_));
      idx_size_ = ex + idx_len_;
    }

    //shift indices
    for (size_t i = idx_len_-1; i>s; i--)
      indice_[i+ex] = indice_[i];
    memcpy(indice_+s+1, str.indice_, str.indice_length()*sizeof(INDEX_));

    //copy str into
    for (size_t i=s+1; i<s+1+str.indice_length(); i++)
    {
      if (COPY_ON_WRITE)
        indice_[i].bptr_->refer();
      else
        duplicate(i);
      
      indice_[i].len_ += last_s;
    }

    idx_len_ += ex;

    if (tail!=NULL)
    {
      indice_[s+1+str.indice_length()].bptr_ = tail;
      indice_[s+1+str.indice_length()].len_ = indice_[s+str.indice_length()].len_ + tt;
    }

    for (size_t i=s+ex+1; i<idx_len_; i++)
      indice_[i].len_ += str.length();

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
    uint64_t i = p - begin();
    insert(i, 1, c);
    return iterator(begin()+i+1);
  }
  
  void insert ( iterator p, size_t n, CharT c )
  {
    uint64_t i = p - begin();
    
    insert(i, n, c);
    return iterator(begin()+i+1);
  }
  
  template<class InputIterator>
  void insert( iterator p, InputIterator first, InputIterator last )
  {
    uint64_t i = p - begin();
    SelfT s;
    s.assign(first, last);
    
    insert(i, s);
  }

  SelfT& erase ( size_t pos = 0, size_t n = npos )
  {
    if (pos>=length_ || n==0)
      return *this;

    if (n == npos)
      n = length_ -pos;

    length_ -= n;
    size_t i = binary_search(pos, 0, idx_len_-1);
    size_t e = binary_search(pos+n-1, i, idx_len_-1);
    size_t il = i==0?0 : indice_[i-1].len_;
    size_t el = e==0?0 : indice_[e-1].len_;

    if (i == e)
    {
      if (is_refered(i))
        duplicate(i);

      size_t j=pos+n-il;
      for (; j<indice_[i].len_-il;j++ )
        (*indice_[i].bptr_)[j-n] = (*indice_[i].bptr_)[j];
      (*indice_[i].bptr_)[j-n] = '\0';

      for (size_t j=i; j<idx_len_; j++)
        indice_[j].len_ -= n;
      
      return *this;
    }

    size_t si = pos - il;
    size_t ei = pos+n-1 - el;

    if (si>0)
    {
      if (is_refered(i))
      {
        duplicate(i);
      }

      indice_[i].bptr_->p_[si] = '\0';
      indice_[i].len_ = il + si;
      i++;
    }

    if (ei+1 < indice_[e].len_-el)
    {
      if (is_refered(i))
      {
        duplicate(i);
      }

      for (size_t j=ei+1; j<indice_[e].len_-el; j++)
        (*indice_[e].bptr_)[j-ei-1] = (*indice_[e].bptr_)[j];
      
      (*indice_[e].bptr_)[indice_[e].len_-el-ei-1-1] = '\0';
      
      e--;
    }

    for (size_t j=i; j<=e; j++)
        derefer(j);

    for (size_t j=e+1; j<idx_len_; j++)
      indice_[j].len_ -= n;

    for (size_t j=e+1; j<idx_len_; j++)
    {
      indice_[j - (e-i+1)] = indice_[j];
      indice_[j].bptr_ = NULL;
    }

    idx_len_ -= (e-i+1);
    
    return *this;
  }
  
  iterator erase ( iterator position )
  {
    size_t i = position - begin();
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
  

  SelfT& replace ( size_t pos1, size_t n1,   const SelfT& str )
  {
    erase(pos1, n1);

    if (str.length()==0)
      return *this;

    if (pos1>1)
      insert(pos1-1, str);
    else
      push_front(str);
    
    return *this;
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
    if (pos + n >= length_)
      n = length_-pos;
    
    size_t i = binary_search(pos, 0, idx_len_-1);
    size_t e = binary_search(pos+n-1, i, idx_len_-1);
    size_t il = i==0?0 : indice_[i-1].len_;
    size_t si = pos - il;

    if (e == i)
    {
      for (size_t j=0; j<n; j++)
      {
        s[j] = indice_[i].bptr_->p_[j+si];
      }
      
      return n;

    }

    for (size_t j=i,k=0; j<=e; j++)
    {
      size_t t = 0;
      if (j==i)
        t = si;

      for (; indice_[i].bptr_->p_[t]!='\0' && k<n; t++,k++)
          s[k] = indice_[j].bptr_->p_[t];
      
    }
    
    return n;
  }

  void swap ( SelfT& str )
  {
    SelfT s = str;
    str = *this;
    *this = s;
  }

  //******************String operations********************
public:

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
    size_t r = 0;
    
    for (const_iterator i =begin()+pos; i<end(); i++, r++)
      if (*i==c)
        return r+pos;
    
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

    if (pos == npos)
      pos = length_ -1;
    
    size_t r = pos;
    for (const_reverse_iterator i =rbegin()+(length_-1-pos); i<rend(); i++, r--)
      if (*i==c)
        return r;
    
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
    const_iterator i=str.begin();
    const_iterator j=begin();
    for (; i!=str.end() && j!=end(); i++,j++)
    {
      if (*j>*i)
        return 1;
      if (*j<*i)
        return -1;
    }

    if (i==str.end() && j==end())
      return 0;
    
    if (j== end())
      return -1;

    return 1;
  }
  
  int compare ( const CharT* s ) const
  {
    size_t len = getLen(s);
    
    size_t i=0;
    const_iterator it = begin();
    
    for (; it!=end() && i<len; i++, it++)
    {
      //std::cout<<*it<<"  "<<s[i]<<std::endl;
      if (*it>s[i])
        return 1;
      if (*it<s[i])
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
    SelfT str(s, n2);
    return substr(*this, pos1, n1).compare(str);
  }

  std::vector<CharT> cast_std_vector()
  {
    std::vector<CharT> v;
    v.resize(length_);
    for (const_iterator i =begin();i!=end(); i++)
      v.push_back(*i);

    return v;
  }
  
  std::string cast_std_string()
  {
    std::string v;
    v.reserve(length_);
    for (const_iterator i =begin();i!=end(); i++)
      v += (*i);

    return v;

  }

  
//******************Serialization********************
public:    
  
  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version)  const
  {
    ar & length_;
    ar & idx_len_;
    ar & idx_size_;

    for (size_t i=0; i<idx_len_; i++)
    {
      ar & indice_[i].len_;
      ar.save_binary(indice_[i].bptr_, BUCKET_SIZE);
    }
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
    clear();
    
    ar & length_;
    ar & idx_len_;
    ar & idx_size_;

    indice_ = (INDEX_*)hlmalloc(idx_size_*sizeof(INDEX_));
    clear_indice(0, idx_size_);
    
    for (size_t i=0; i<idx_len_; i++)
    {
      ar & indice_[i].len_;
      indice_[i].bptr_ = new_bucket();
      ar.load_binary(indice_[i].bptr_, BUCKET_SIZE);
    }
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

friend std::ostream& operator << (std::ostream& os, const SelfT& str)
  {
    for (const_iterator i =str.begin(); i!=str.end(); i++)
      os<<*i;

    return os;
  }

  void display() const
  {
    for (size_t i=0; i<idx_len_; i++)
    {
      std::cout<<indice_[i]<<std::endl;
    }
  }
  
}
  ;

template <
  class CHAR_TYPE ,
  int COPY_ON_WRITE,
  uint64_t BUCKET_BYTES
  >
const size_t bbt_string<CHAR_TYPE ,COPY_ON_WRITE, BUCKET_BYTES>::npos = -1;

NS_IZENELIB_AM_END
#endif

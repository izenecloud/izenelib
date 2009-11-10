#ifndef INTEGER_DYN_ARRAY_HPP
#define INTEGER_DYN_ARRAY_HPP

#include<types.h>
#include <vector>
#include <ostream>
#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

NS_IZENELIB_AM_BEGIN


template<
  class INTEGER_TYPE = unsigned int,
  bool  AUTO_SORT = false,
  uint8_t APPEND_RATE = 1, // POWER OF 2
  bool COPY_ON_WRITE = true,
  class LENG_TYPE = uint32_t
  >
class IntegerDynArray
{
  typedef uint8_t  ReferT;
  typedef IntegerDynArray<INTEGER_TYPE, AUTO_SORT, APPEND_RATE, COPY_ON_WRITE> SelfT;
public:
  typedef LENG_TYPE size_t;
  
private:
  char* p_;
  INTEGER_TYPE* array_;
  size_t length_;
  size_t max_size_;

protected:
  inline void refer()
  {
    if (!COPY_ON_WRITE)
      return ;
        
    if (p_!=NULL)
    {
      if (*(ReferT*)p_==(ReferT)-1)
      {
        char* p = (char*)malloc(length_*sizeof(INTEGER_TYPE)+sizeof(ReferT));
        memcpy(p + sizeof (ReferT), array_, size());
        array_ = (INTEGER_TYPE*)(p+sizeof (ReferT));
          
        p_ = p;
        (*(ReferT*)p_) = 1;
        return;
      }
      (*(ReferT*)p_)++;
      
    }

  }

  inline void derefer()
  {
    if (!COPY_ON_WRITE)
    {
      if (p_!=NULL){
        free(p_);
        p_ = NULL;
        array_ = NULL;
      }
      
      return ;
    }

    if (p_!=NULL && *(ReferT*)p_ > 0)
    {
      //boost::mutex::scoped_lock lock(mutex_);
      //if (p_!=NULL && *(ReferT*)p_ > 0)
        (*(ReferT*)p_)--;
      if (*(ReferT*)p_== 0)
      {
        //std::cout<<"free me!\n";
        free(p_);
        p_ = NULL;
        array_ = NULL;
        //std::cout<<"alright\n";
      }
    }

  }

  inline bool is_refered()const
  {
    if (!COPY_ON_WRITE)
      return false;

    if (p_!= NULL && *(ReferT*)p_ > 1)
    {
      //boost::mutex::scoped_lock lock(mutex_);
      //if (p_!= NULL && *(ReferT*)p_ > 1)
        return true;
    }

    return false;
  }

  inline void clean_reference()
  {
    if (!COPY_ON_WRITE)
      return ;
    
    if (p_!=NULL)
    {
      //boost::mutex::scoped_lock lock(mutex_);
      //if (p_!=NULL)
        (*(ReferT*)p_) = 1;
    }
    
  }

  inline void assign_self()
  {
    //std::cout<<"assign_self............\n";
    char* p = (char*)malloc(length_*sizeof(INTEGER_TYPE)+sizeof(ReferT));
    max_size_ = length_;
    memcpy(p + sizeof (ReferT), array_, capacity());
    array_ = (INTEGER_TYPE*)(p+sizeof (ReferT));
    
    derefer();
    p_ = p;
    clean_reference(); 
  }

  inline void new_one(size_t t)
  {
    length_ = max_size_ = t;
    p_ = (char*)malloc(t*sizeof(INTEGER_TYPE)+sizeof(ReferT));
    array_ = (INTEGER_TYPE*)(p_+sizeof(ReferT));
    clean_reference();
  }

  inline void enlarge(size_t t)
  {
    max_size_ = t;
    p_ = (char*)realloc(p_, t*sizeof(INTEGER_TYPE)+sizeof(ReferT));
    array_ = (INTEGER_TYPE*)(p_+sizeof(ReferT));
    clean_reference();
  }

  inline void shrink()
  {
    max_size_ = length_;
    p_ = (char*)realloc(p_, length_*sizeof(INTEGER_TYPE)+sizeof(ReferT));
    array_ = (INTEGER_TYPE*)(p_+sizeof(ReferT));
    clean_reference();
  }

  inline size_t binary_search(INTEGER_TYPE t, size_t low, size_t high)const
  {
    size_t mid;
    
    if (t> array_[length_-1])
      return length_-1;
    if (t < array_[0])
      return -1;
    
    while (low < high) {
      mid = low + ((high - low) / 2);  // Note: not (low + high) / 2 !!
        if (array_[mid] < t)
          low = mid + 1; 
        else
          //can't be high = mid-1: here A[mid] >= value,
          //so high can't be < mid if A[mid] == value
          high = mid; 
      }
    // high == low, using high or low depends on taste
    if (array_[low]<=t)
      return low;
    else
      return low-1;
  }

  inline size_t find_pos_2insert(INTEGER_TYPE t)const
  {
    if (length_ ==0)
      return -1;
    
    if (!AUTO_SORT)
      return length_-1;

    return binary_search(t, 0, length_-1);
  }
  
public:
  inline explicit IntegerDynArray()
    :p_(NULL), array_(NULL), length_(0), max_size_(0)
  {
  }

  inline explicit IntegerDynArray(const std::vector<INTEGER_TYPE>& v)
    :p_(NULL), array_(NULL), length_(0), max_size_(0)
  {
    assign(v);
  }

  inline IntegerDynArray(const char* p, size_t len)
    :p_(NULL), array_(NULL), length_(0), max_size_(0)
  {
    assign(p, len);
  }

  inline IntegerDynArray(const SelfT& other)
    :p_(NULL), array_(NULL), length_(0), max_size_(0)
  {
    assign(other);
  }

  inline explicit IntegerDynArray(size_t n)
    :p_(NULL), array_(NULL), length_(0), max_size_(0)
  {
    reserve(n);
  }

  inline ~IntegerDynArray()
  {
    derefer();
  }
  
  inline void assign(const std::vector<INTEGER_TYPE>& v)
  {
    if (v.size()==0)
    {
      clear();
      return;
    }

    derefer();
    
    new_one(v.size());
    memcpy(array_, v.data(), sizeof(INTEGER_TYPE)*v.size());
  }
  
  inline void assign(const char* p, size_t len)
  {
    if (!len)
    {
      clear();
      return;
    }

    derefer();
    
    new_one(len/sizeof(INTEGER_TYPE));
    memcpy(array_, p, len);
  }

  inline void assign(const SelfT& other)
  {
    IASSERT(this!=&other);
    
    if (other.length()==0)
    {
      clear();
      return;
    }

    derefer();

    if (COPY_ON_WRITE)
    {
      p_ = other.p_;
      array_ = other.array_;
      length_ = other.length_;
      max_size_ = other.max_size_;

      refer();
      return;
    }

    new_one(other.length());
    memcpy(array_, other.array_, sizeof(INTEGER_TYPE)*other.length());
  }

  inline SelfT& operator = (const SelfT& other)
  {
    assign(other);
    return *this;
  }

  SelfT& operator += (const SelfT& other)
  {
    IASSERT(array_!= other.array_);
    
    if (other.length()==0)
      return *this;

    if (is_refered())
      assign_self();

    if (max_size_ < length_+other.length())
      enlarge(length_+other.length());

    memcpy(&array_[length_], other.array_, other.size());
    length_ += other.length();

    return *this;
  }

  SelfT& operator += (const std::vector<INTEGER_TYPE>& other)
  {
    if (other.size()==0)
      return *this;

    if (is_refered())
      assign_self();

    if (max_size_ < length_+other.size())
      enlarge(length_+other.size());
    
    memcpy(&array_[length_], other.data(), other.size()*sizeof(INTEGER_TYPE));
    
    length_ += other.size();
    
    return *this;
  }
  
  inline SelfT& operator += (INTEGER_TYPE t)
  {
    push_back(t);
    return *this;
  }

  inline bool operator == (const SelfT& other)
  {
    if (length_ != other.length())
      return false;

    for (size_t i=0; i<length_; i++)
      if (at(i)!= other.at(i))
        return false;
    return true;
  }

  inline bool operator == (const INTEGER_TYPE* other)
  {
    for (size_t i=0; i<length_; i++)
      if (at(i)!= other[i])
        return false;

    return true;
  }
  
  inline void push_back(INTEGER_TYPE t)
  {
    if (is_refered())
      assign_self();
    
    size_t n = find_pos_2insert(t);
    insert(n, t);    
  }
  
  inline INTEGER_TYPE pop_back()
  {
    if (is_refered())
      assign_self();

    INTEGER_TYPE r = at(length_-1);
    --length_;
    return r;
  }

  inline bool add_tail(INTEGER_TYPE t)
  {
    // if (max_size_==0 || max_size_ == length_)
//       enlarge((max_size_+1)<<APPEND_RATE);
    
    IASSERT(length_<max_size_);
    array_[length_] = t;
    ++length_;
    return true;
  }

  void erase(size_t t)
  {
    IASSERT(t<length_);
    
    if (is_refered())
      assign_self();
    
    for (size_t i=t; i<length_-1; i++)
      array_[i] = array_[i+1];
    length_--;
  }

  inline void compact()
  {
    if (max_size_ <= length_+1)
      return;
    
    if (is_refered())
      return;

    if (length_ == 0)
    {
      clear();
      return;
    }

    shrink();
  }

  inline void clear()
  {
    derefer();
    p_ = NULL;
    max_size_ = length_ = 0;
    array_ = NULL;
  }  

  inline void insert(size_t n, INTEGER_TYPE t)
  {
    IASSERT(n < length_ || n == (size_t)-1);
    if (is_refered())
      assign_self();

    if (max_size_==0 || max_size_ == length_)
      enlarge((max_size_+1)<<APPEND_RATE);
    
    for (size_t i=length_; i>n+1; i--)
      array_[i] = array_[i-1];
    array_[n+1] = t;
    length_++;
  }

  size_t find(INTEGER_TYPE t)const
  {
    if (!AUTO_SORT)
    {
      for (size_t i=0; i<length_; i++)
        if (array_[i]==t)
          return i;

      return -1;
    }

    size_t n = binary_search(t, 0, length_-1);
    if (n == (size_t)-1 || array_[n]!=t)
      return -1;

    return n;
  }

  inline INTEGER_TYPE at (size_t t)const
  {
    IASSERT(t < length_);
    return array_[t];
  }
  
  inline INTEGER_TYPE& operator [] (size_t t)
  {
    IASSERT(t < length_);
    
     if (is_refered())
       assign_self();

    return array_[t];
  }

  inline const INTEGER_TYPE& operator [] (size_t t)const
  {
    IASSERT(t < length_);
    return array_[t];
  }

  inline void reserve(size_t n)
  {
    if (n <= max_size_)
      return;

    if (is_refered())
      assign_self();

    enlarge(n);
  }

  inline void reset()
  {
    length_ = 0;
  }

  inline INTEGER_TYPE* array(size_t n)
  {
    reserve(n);
    length_ = n;
    return array_;
  }
  
  inline size_t length()const
  {
    return length_;
  }

  inline size_t capacity()const
  {
    return sizeof(INTEGER_TYPE) * max_size_;
  }

  inline size_t size()const
  {
    return sizeof(INTEGER_TYPE) * length_;
  }

  inline INTEGER_TYPE* data()const
  {
    return array_;
  }
  

  bool operator == (const SelfT& v)const
  {
    if (length_ != v.length())
      return false;

    for (size_t i=0; i<length_; i++)
      if (array_[i]!=v.array_[i])
        return false;
    return true;
  }

  bool operator == (const std::vector<INTEGER_TYPE>& v)
  {
    if (length_ != v.size())
      return false;

    for (size_t i=0; i<length_; i++)
      if (array_[i]!=v[i])
        return false;
    return true;
  }
  
  

  
  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const SelfT& v)
  {
    for (size_t i =0; i<v.length_; i++)
      os<<v.array_[i]<<" ";

    return os;
  }


  /**
   *This is the interface for boost::serialization. Make it serializable by boost.
   **/
  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version)  const
  {
    ar & length_;
    ar.save_binary(array_, size());
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
    derefer();
    
    ar & length_;
    max_size_ = length_;
    new_one(length_);
    
    ar.load_binary(array_, size());
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
  
  
}
  ;

NS_IZENELIB_AM_END
#endif

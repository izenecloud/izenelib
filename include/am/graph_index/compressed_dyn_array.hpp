/**
   @file dyn_arry.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef COMPRESSED_DYN_ARRAY_HPP
#define COMPRESSED_DYN_ARRAY_HPP

#include<types.h>
#include <vector>
#include <ostream>
#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

NS_IZENELIB_AM_BEGIN

/**
   @class DynArray
   @detail it is a copy-on-write dynamic array, and fix length data only.
 */
template<
  class LONG_VALUE_TYPE = unsigned int,
  class SHORT_VALUE_TYPE = unsigned short,
  bool  AUTO_SORT = false,
  uint8_t APPEND_RATE = 1, // POWER OF 2
  bool COPY_ON_WRITE = true,
  class LENG_TYPE = uint32_t
  >
class DynArray
{
  typedef uint8_t  ReferT;
  typedef DynArray<LONG_VALUE_TYPE, SHORT_VALUE_TYPE, AUTO_SORT, APPEND_RATE, COPY_ON_WRITE, LENG_TYPE> SelfT;
public:
  typedef LENG_TYPE size_t;
  static const size_t NOT_FOUND = -1;

#define ARRAY(p) ((uint8_t*)((p)+sizeof(ReferT)))
#define REFER (*(ReferT*)p_)
#define LONG(p, i) ((ARRAY(p) & 128)>0) 
private:
  char* p_;//!< buffer for array
  size_t length_;//!< vector size
  size_t max_size_;//!< buffer size.

protected:
  /**
     @brief add reference count by 1
   */
  inline void refer()
  {
    if (!COPY_ON_WRITE)
      return ;
        
    if (p_!=NULL)
    {
      if (REFER ==(ReferT)-1)
      {
        char* p = (char*)malloc(size()+sizeof(ReferT));
        memcpy((char*)ARRAY(p), (char*)ARRAY(p_), size());
          
        p_ = p;
        REFER = 1;
        return;
      }
      (*(ReferT*)p_)++;
      
    }

  }

  /**
     @brief deduct reference count by 1.
   */
  inline void derefer()
  {
    if (!COPY_ON_WRITE)
    {
      if (p_!=NULL){
        free(p_);
        p_ = NULL;
      }
      
      return ;
    }

    if (p_!=NULL && REFER > 0)
    {
      //boost::mutex::scoped_lock lock(mutex_);
      //if (p_!=NULL && *(ReferT*)p_ > 0)

      REFER--;
      if (REFER == 0)
      {
        //std::cout<<"free me!\n";
        free(p_);
        p_ = NULL;
        //std::cout<<"alright\n";
      }
    }

  }

  /**
     @brief check if reference count is equal to 1
     @return true reference count is large than 1, refered
   */
  inline bool is_refered()const
  {
    if (!COPY_ON_WRITE)
      return false;

    if (p_!= NULL && REFER > 1)
    {
        return true;
    }

    return false;
  }

  /**
     @brief clean reference count
   */
  inline void clean_reference()
  {
    if (!COPY_ON_WRITE)
      return ;
    
    if (p_!=NULL)
    {
      REFER = 1;
    }
    
  }

  /**
     @brief make one copy when write to a refered vector
   */
  inline void assign_self()
  {
    //std::cout<<"assign_self............\n";
    char* p = (char*)malloc(size()+sizeof(ReferT));
    max_size_ = size();
    memcpy((char*)ARRAY(p), ARRAY(p_), size());
    
    derefer();
    p_ = p;
    clean_reference(); 
  }

  inline void new_one(size_t t)
  {
    length_ = t;
    max_size_ = t*sizeof(LONG_VALUE_TYPE);
    p_ = (char*)malloc(size()+sizeof(ReferT));
    clean_reference();
  }

  inline void enlarge(size_t t)
  {
    max_size_ = t*sizeof(LONG_VALUE_TYPE);
    p_ = (char*)realloc(p_, size()+sizeof(ReferT));

    clean_reference();
  }

  inline void shrink()
  {
    size_t j=0;
    for (size_t i=0; i<length_; ++i, j+= sizeof (SHORT_VALUE_TYPE))
      if (LONG(p_, j))
        j+= sizeof (SHORT_VALUE_TYPE)
    
    max_size_ = j;
    p_ = (char*)realloc(p_, size()+sizeof(ReferT));

    clean_reference();
  }

  inline size_t size_of_(LONG_VALUE_TYPE t)const
  {        
    static const SHORT_VALUE_ max_short = ((SHORT_VALUE_TYPE)(-1))>>1;
    static const LONG_VALUE_TYPE max_long = ((LONG_VALUE_TYPE)(-1))>>1;

    if (t>max_long)
    {
      std::cout<<t<<" too long\n";
      return sizeof(LONG_VALUE_TYPE);
    }
    
    if (t>max_short)
    {
      return sizeof(LONG_VALUE_TYPE);
    }
    else
    {
      return sizeof(SHORT_VALUE_TYPE);
    }

  }

  inline LONG_VALUE_TYPE flag_()const
  {
    static const LONG_VALUE_TYPE flag = ((LONG_VALUE_TYPE)(1))<<sizeof(LONG_VALUE_TYPE)*8-1;
    return flag;
  }
  
  
//   inline size_t binary_search(VALUE_TYPE t, size_t low, size_t high)const
//   {
//     size_t mid;
    
//     if (t> ARRAY(p_)[length_-1])
//       return length_-1;
//     if (t < ARRAY(p_)[0])
//       return -1;
    
//     while (low < high) {
//       mid = low + ((high - low) / 2);  // Note: not (low + high) / 2 !!
//         if (ARRAY(p_)[mid] < t)
//           low = mid + 1; 
//         else
//           //can't be high = mid-1: here A[mid] >= value,
//           //so high can't be < mid if A[mid] == value
//           high = mid; 
//       }
//     // high == low, using high or low depends on taste
//     if (ARRAY(p_)[low]<=t)
//       return low;
//     else
//       return low-1;
//   }

  inline size_t find_pos_2insert(VALUE_TYPE t)const
  {
    if (length_ ==0)
      return -1;
    
    if (!AUTO_SORT)
      return length_-1;

    return length_-1;//binary_search(t, 0, length_-1);
  }

  
//    /**
//      Swap two elements.
//   **/
//   void swap_(VALUE_TYPE* a, VALUE_TYPE* b)
//   {
//     VALUE_TYPE temp;
//     temp = *a;
//     *a = *b;
//     *b = temp;
//   }

//   /**
//      When quick sort, it return the index of media element.
//    **/
//   size_t findMedianIndex_(VALUE_TYPE* array, size_t left, size_t right)
//   {
//     return (left+right)/2;
    
// //     VALUE_TYPE min = array[left];
// //     VALUE_TYPE max = array[left];
// //     size_t shift = 5;

// //     if (right-left<2*shift)
// //       return (left+right)/2;
    
// //     for (size_t i=left+shift; i<=right; i+=shift)
// //     {
// //       if (array[i]<min)
// //       {
// //         min = array[i];
// //         continue;
// //       }

// //       if (array[i] > max)
// //       {
// //         max = array[i];
// //         continue;
// //       }
// //     }

// //     VALUE_TYPE average = (min + max )/2;
// //     size_t idx = left;
// //     VALUE_TYPE minGap = average>array[idx]? average-array[idx] : array[idx]-average;
    
// //     for (size_t i=left+shift; i<=right; i+=shift)
// //     {
// //       VALUE_TYPE gap = average>array[i]? average-array[i] : array[i]-average;
// //       if (gap<minGap)
// //       {
// //         minGap = gap;
// //         idx = i;
// //       }
// //     }

// //     return idx;
    
//   }
  
//   /**
//      Partition the array into two halves and return the index about which the array is partitioned.
//   **/
//   size_t partition_(VALUE_TYPE* array, size_t left, size_t right)
//   {
//     size_t pivotIndex = findMedianIndex_(array, left, right), index = left, i;
//     VALUE_TYPE pivotValue = array[pivotIndex];
 
//     swap_(&array[pivotIndex], &array[right]);
    
//     for(i = left; i <right; i++)
//     {
//       if(array[i] <= pivotValue)
//       {
//         swap_(&array[i], &array[index]);
//         index += 1;
//       }
//     }
//     swap_(&array[right], &array[index]);
 
//     return index;
//   }

//   /**
//      A recursive function applying quick sort.
//    **/
//   void quickSort_(VALUE_TYPE* array, size_t left, size_t right)
//   {
//     IASSERT(left < right);

//     IASSERT(right < length_);
    
//     if(right-left<=1)
//     {
//       if (array[left]>array[right])
//         swap_(&array[left], &array[right]);
//       return;
//     }
    
//     size_t idx = partition_(array, left, right);
    
//     if(idx>0 && left<idx-1)
//       quickSort_(array, left, idx - 1);
//     if (idx+1<length_ && idx+1<right)
//       quickSort_(array, idx + 1, right);
//   }

public:
  
  inline explicit DynArray()
    :p_(NULL), length_(0), max_size_(0)
  {
  }

  /**
     @brief it can be initialized with std::vector
   */
  inline explicit DynArray(const std::vector<LONG_VALUE_TYPE>& v)
    :p_(NULL), length_(0), max_size_(0)
  {
    assign(v);
  }

  /**
     @brief initialized with a buffer
   */
  DynArray(const char* p, size_t len)
    :p_(NULL), length_(0), max_size_(0)
  {
    assign(p, len);
  }

  /**
     @brief a copy constructor
   */
  DynArray(const SelfT& other)
    :p_(NULL), length_(0), max_size_(0)
  {
    assign(other);
  }

  /**
     @brief reserve memory of n data
   */
  explicit DynArray(size_t n)
    :p_(NULL), length_(0), max_size_(0)
  {
    reserve(n);
  }

  inline ~DynArray()
  {
    derefer();
  }

  /**
     @brief it can be initialized with std::vector
   */
  inline void assign(const std::vector<LONG_VALUE_TYPE>& v)
  {
    if (v.size()==0)
    {
      clear();
      return;
    }

    derefer();
    
    new_one(v.size());
    for (size_t i=0, j=0; i<v.size(); ++i)
    {
      if (size_of_(v[i]) == sizeof(LONG_VALUE_TYPE))
      { 
        *((LONG_VALUE_TYPE*)(ARRAY(p_)+j)) = v[i]|flag_();
        j += sizeof(LONG_VALUE_TYPE);
      }
      else
      {
        *((SHORT_VALUE_TYPE*)(ARRAY(p_)+j)) = v[i];
        j += sizeof(SHORT_VALUE_TYPE);
      }
    }
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
      length_ = other.length_;
      max_size_ = other.max_size_;

      refer();
      return;
    }

    new_one(other.length());
    memcpy(ARRAY(p_), ARRAY(other.p_), size());
  }

  inline SelfT& operator = (const SelfT& other)
  {
    assign(other);
    return *this;
  }
  
  inline bool operator == (const SelfT& other)
  {
    if (length()!=other.length())
      return false;
    
    size_t i=0;
    for (; i<length(); ++i)
      if (at(i) != other.at(i))
        return false;

    return true;
  }
  
  inline bool operator < (const SelfT& other)
  {
    size_t i=0;
    for (; i<length()&&i<other.length(); ++i)
      if (at(i)<other.at(i))
        return true;
      else if (at(i)>other.at(i))
        return false;

    if (length()<other.length())
      return true;
    
    return false;
  }
  
  inline bool operator > (const SelfT& other)
  {
    size_t i=0;
    for (; i<length()&&i<other.length(); ++i)
      if (at(i)>other.at(i))
        return true;
      else if (at(i)<other.at(i))
        return false;

    if (length()>other.length())
      return true;
    
    return false;
  }

  inline bool operator <= (const SelfT& other)
  {
    return *this<other || *this == other;
  }

  inline bool operator >= (const SelfT& other)
  {
    return *this>other || *this == other;
  }
  
  inline SelfT& operator = (const std::vector<LONG_VALUE_TYPE>& other)
  {
    assign(other);
    return *this;
  }

  /**
     @brief append another vector at the end
   */
  SelfT& operator += (const SelfT& other)
  {
    IASSERT(p_!= other.p_);
    
    if (other.length()==0)
      return *this;

    if (is_refered())
      assign_self();

    if (max_size_ < length_+other.length())
      enlarge(length_+other.length());

    memcpy(ARRAY(p_)+size(), ARRAY(other.p_), other.size());
    length_ += other.length();

    return *this;
  }

  /**
     @brief Append a std::vector at the end
   */
  SelfT& operator += (const std::vector<LONG_VALUE_TYPE>& other)
  {
    if (other.size()==0)
      return *this;

    if (is_refered())
      assign_self();

    SelfT tmp(other);
    return operator +=(tmp);
  }

  /**
     @brief appended by a element.
   */
  inline SelfT& operator += (LONG_VALUE_TYPE t)
  {
    push_back(t);
    return *this;
  }

  inline bool operator == (const LONG_VALUE_TYPE* other)
  {
    for (size_t i=0; i<length_; i++)
      if (at(i)!= other[i])
        return false;

    return true;
  }

  /**
     @see inline SelfT& operator += (VALUE_TYPE t)
   */
  inline size_t push_back(LONG_VALUE_TYPE t)
  {
    if (is_refered())
      assign_self();
    
    size_t n = find_pos_2insert(t);
    insert(n, t);
    
    return n+1;
  }

  /**
     @brief remove the last element
   */
  inline LONG_VALUE_TYPE pop_back()
  {
    if (length_ ==0)
      return LONG_VALUE_TYPE();
    
    if (is_refered())
      assign_self();
    
    LONG_VALUE_TYPE r = at(length_-1);
    --length_;
    return r;
  }

  /**
     @brief it's equal to push_back(), but make sure memory has been
     reserved and no reference before calling.
   */
  inline bool add_tail(LONG_VALUE_TYPE t)
  {
    // if (max_size_==0 || max_size_ == length_)
//       enlarge((max_size_+1)<<APPEND_RATE);
    
    IASSERT(length_<max_size_);
    
    if (size_of_(t) == sizeof(LONG_VALUE_TYPE))
    {
      *((LONG_VALUE_TYPE*)(ARRAY(p_)+size())) = t | flag_();
      size_ += sizeof(LONG_VALUE_TYPE);
    }
    else
    {
      *((SHORT_VALUE_TYPE*)(ARRAY(p_)+size())) = t;
      size_ += sizeof(SHORT_VALUE_TYPE);
    }
    
    ++length_;
    return true;
  }

  /**
     @brief remove an element indicated by index t.
   */
  void erase(size_t t)
  {
    IASSERT(t<length_);
    
    if (is_refered())
      assign_self();

    size_t i = 0;
    for (size_t j; j<t; ++j)
      if (LONG(p_, i))
        i += sizeof(LONG_VALUE_TYPE);
      else
        i += sizeof(SHORT_VALUE_TYPE);

    size_t gap = (LONG(p_, i))? sizeof(LONG_VALUE_TYPE): sizeof(SHORT_VALUE_TYPE);
    for (; i<size()-gap; i++)
      ARRAY(p_)[i] = ARRAY(p_)[i+gap];
    length_--;
    size_ -= gap;
  }

  /**
     @brief compact vector
   */
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

  /**
     @brief deallocate
   */
  inline void clear()
  {
    derefer();
    p_ = NULL;
    size_ = max_size_ = length_ = 0;
  }  

  /**
     @brief insert t at n.
   */
  inline void insert(size_t n, LONG_VALUE_TYPE t)
  {
    IASSERT(n < length_ || n == NOT_FOUND);
    if (is_refered())
      assign_self();

    IASSERT(max_size_>=length_);
    
    while (size_==0 || max_size_ <= size() + sizeof(LONG_VALUE_TYPE))
      enlarge((size_t)((max_size_+1)*1.2+.5));//<<APPEND_RATE);

    size_t gap = size_of_(t);
    for (size_t i=size_-1; i>n+1; i--)
      ARRAY(p_)[i+gap] = ARRAY(p_)[i-1];
    
    IASSERT(n+1<=length_ && n+1<max_size_);
    ARRAY(p_)[n+1] = t;
    length_++;
  }

  size_t find(VALUE_TYPE t)const
  {
    if (length_ == 0)
      return NOT_FOUND;
    
    if (!AUTO_SORT)
    {
      for (size_t i=0; i<length_; i++)
        if (ARRAY(p_)[i]==t)
          return i;

      return NOT_FOUND;
    }

    size_t n = binary_search(t, 0, length_-1);
    if (n == NOT_FOUND || ARRAY(p_)[n]!=t)
      return NOT_FOUND;

    return n;
  }

  inline VALUE_TYPE at (size_t t)const
  {
    IASSERT(t < length_);
    return ARRAY(p_)[t];
  }

  inline VALUE_TYPE back ()const
  {
    IASSERT(length_>0);
    return at(length_ -1);
  }

  inline VALUE_TYPE front ()const
  {
    IASSERT(length_>0);
    return at(0);
  }
  
  inline VALUE_TYPE& operator [] (size_t t)
  {
    IASSERT(t < length_);
    
     if (is_refered())
       assign_self();

    return ARRAY(p_)[t];
  }

  inline const VALUE_TYPE& operator [] (size_t t)const
  {
    IASSERT(t < length_);
    return ARRAY(p_)[t];
  }

  inline void reserve(size_t n)
  {
    if (n <= max_size_)
      return;

    if (is_refered())
      assign_self();

    enlarge(n);
  }

  /**
     @brief set length to 0.
   */
  inline void reset()
  {
    length_ = 0;
  }

  /**
     @breif except for returning head pointer of buffer, it reserve memory for n elements
     @return head pointer of buffer.
   */
  inline VALUE_TYPE* array(size_t n)
  {
    reserve(n);
    length_ = n;
    return ARRAY(p_);
  }

  /**
     @return the number of element.
   */
  inline size_t length()const
  {
    return length_;
  }

  /**
     @return the entire memory size it takes.
   */
  inline size_t capacity()const
  {
    return sizeof(VALUE_TYPE) * max_size_;
  }

  /**
     @brief the memory size.
   */
  inline size_t size()const
  {
    return sizeof(VALUE_TYPE) * length_;
  }

  /**
     @brief it gives the pointer of buffer.
   */
  inline VALUE_TYPE* data()const
  {
    return ARRAY(p_);
  }
  

  bool operator == (const SelfT& v)const
  {
    if (length_ != v.length())
      return false;

    for (size_t i=0; i<length_; i++)
      if (ARRAY(p_)[i]!=ARRAY(v.p_)[i])
        return false;
    return true;
  }

  bool operator == (const std::vector<VALUE_TYPE>& v)
  {
    if (length_ != v.size())
      return false;

    for (size_t i=0; i<length_; i++)
      if (ARRAY(p_)[i]!=v[i])
        return false;
    return true;
  }

  /**
     @brief sort vector in 
   */
  void sort()
  {
    if (length_<=1 || AUTO_SORT)
      return;

    if (is_refered())
      assign_self();

    quickSort_(ARRAY(p_), 0, length_-1);
  }  

//   void merge_sort()
//   {
//     if (length_<=1 || AUTO_SORT)
//       return;

//     if (is_refered())
//       assign_self();

//     const size_t SIZE = 1000000;
//     if (length_/SIZE <=2)
//       return quickSort_(ARRAY(p_), 0, length_-1);

//     const size_t LEN = length_%SIZE==0? length_/SIZE: length_/SIZE+1;
//     size_t* index = (size_t*)malloc(sizeof(size_t)*LEN);
//     for (size_t i=0; i<LEN; ++i)
//       index[i] = 0;
    
//     size_t i=0;
//     for (; i<length_; i+=SIZE)
//       quickSort_(ARRAY(p_), i, ((i+SIZE-1)>length_-1? length_-1: (i+SIZE-1)));;

//     char* p = (char*)malloc(length_*sizeof(VALUE_TYPE)+sizeof(ReferT));
//     i = 0;
//     size_t finished = 0;

//     while (finished != LEN)
//     {
//       size_t mini = 0;
//       VALUE_TYPE min;

//       size_t j = 0;
//       for (; j<LEN; ++j)
//         if (index[j]<SIZE)
//         {
//           mini = j;
//           min = *(ARRAY(p_)+j*SIZE+index[j]);
//           ++j;
//           break;
//         }

//       for (; j<LEN; ++j)
//       {
//         if (index[j]>=SIZE)
//           continue;

//         if (min > *(ARRAY(p_)+j*SIZE+index[j]))
//         {
//           min = *(ARRAY(p_)+j*SIZE+index[j]);
//           mini = j;
//         }
//       }

//       *(ARRAY(p)+i) = min;
//       ++i;

//       IASSERT(index[mini]<SIZE);
//       ++index[mini];
//       if (index[mini]==SIZE)
//         ++finished;
//     }

//     free(p_);
//     p_ = p;
//     clean_reference();

//     free(index);
//   }
  
  
  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const SelfT& v)
  {
    for (size_t i =0; i<v.length_; i++)
      os<<ARRAY(v.p_)[i]<<" ";

    return os;
  }

  /**
     @return the size used for saving this vector
   */
  size_t save_size()const
  {
    return sizeof(size_t)+size();
  }
  
  size_t save(FILE* f, uint64_t addr = -1)
  {
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);
    size_t s = size();
    IASSERT(fwrite(&s, sizeof(size_t), 1, f)==1);

    if (s > 0)
      IASSERT(fwrite(ARRAY(p_), s, 1, f)==1);

    return s+sizeof(size_t);
  }
  
  size_t load(FILE* f, uint64_t addr = -1)
  {
    clear();
    
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);
    
    size_t s = 0;

    IASSERT(fread(&s, sizeof(size_t), 1, f)==1);

    max_size_ = length_ = s/sizeof(VALUE_TYPE);

    new_one(length_);

    if (s > 0)
      IASSERT(fread(ARRAY(p_), s, 1, f)==1);

    return s;
  }

  /**
    @brief it load from a memory buffer.
   */
  size_t load(const char* mem)
  {
    clear();
        
    size_t s = *(size_t*)mem;

    max_size_ = length_ = s/sizeof(VALUE_TYPE);

    new_one(length_);

    memcpy(ARRAY(p_), mem+sizeof(size_t), s);

    return s;
  }

  /**
   *This is the interface for boost::serialization. Make it serializable by boost.
   **/
  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version)  const
  {
    ar & length_;
    ar.save_binary(ARRAY(p_), size());
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
    clear();
    //derefer();
    
    ar & length_;
    max_size_ = length_;
    new_one(length_);
    
    ar.load_binary(ARRAY(p_), size());
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
  
  
}
  ;

NS_IZENELIB_AM_END
#endif

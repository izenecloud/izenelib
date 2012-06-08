/**
   @file dyn_arry.hpp
   @author Kevin Hu
   @date 2009.11.24
 */
#ifndef DYN_ARRAY_HPP
#define DYN_ARRAY_HPP
 
#include<types.h>
#include <vector>
#include <ostream>
#include <iostream>
#include <stdio.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

NS_IZENELIB_AM_BEGIN

/**
   @class DynArray
   @detail it is a copy-on-write dynamic array, and fix length data only.
 */
template<
  class VALUE_TYPE = unsigned int,
  bool  AUTO_SORT = false,
  uint8_t APPEND_RATE = 1, // POWER OF 2
  bool COPY_ON_WRITE = true,
  class LENG_TYPE = uint32_t
  >
class DynArray
{
  typedef uint8_t  ReferT;
  typedef DynArray<VALUE_TYPE, AUTO_SORT, APPEND_RATE, COPY_ON_WRITE, LENG_TYPE> SelfT;
public:
  typedef LENG_TYPE size_t;
  static const size_t NOT_FOUND = -1;

#define ARRAY(p) ((VALUE_TYPE*)((p)+sizeof(ReferT)))
#define REFER (*(ReferT*)p_)

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
        char* p = (char*)malloc(length_*sizeof(VALUE_TYPE)+sizeof(ReferT));
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
    char* p = (char*)malloc(length_*sizeof(VALUE_TYPE)+sizeof(ReferT));
    max_size_ = length_;
    memcpy((char*)ARRAY(p), ARRAY(p_), capacity());
    
    derefer();
    p_ = p;
    clean_reference(); 
  }

  inline void new_one(size_t t)
  {
    length_ = max_size_ = t;
    p_ = (char*)malloc(t*sizeof(VALUE_TYPE)+sizeof(ReferT));
    clean_reference();
  }

  inline void enlarge(size_t t)
  {
    max_size_ = t;
    p_ = (char*)realloc(p_, t*sizeof(VALUE_TYPE)+sizeof(ReferT));

    clean_reference();
  }

  inline void shrink()
  {
    max_size_ = length_;
    p_ = (char*)realloc(p_, length_*sizeof(VALUE_TYPE)+sizeof(ReferT));

    clean_reference();
  }

  inline size_t binary_search(VALUE_TYPE t, size_t low, size_t high)const
  {
    if (length_ == 0)
      return -1;

    IASSERT(high>=low);
    
    size_t mid;
    
    if (t> ARRAY(p_)[length_-1])
      return length_-1;
    if (t < ARRAY(p_)[0])
      return -1;
    
    while (low < high) {
      mid = low + ((high - low) / 2);  // Note: not (low + high) / 2 !!
        if (ARRAY(p_)[mid] < t)
          low = mid + 1; 
        else
          //can't be high = mid-1: here A[mid] >= value,
          //so high can't be < mid if A[mid] == value
          high = mid; 
      }
    // high == low, using high or low depends on taste
    if (ARRAY(p_)[low]<=t)
      return low;
    else
      return low-1;
  }

public:
  inline size_t find_pos_2insert(VALUE_TYPE t)const
  {
    if (length_ ==0)
      return -1;
    
    if (!AUTO_SORT)
      return length_-1;

    return binary_search(t, 0, length_-1);
  }

  inline size_t find_pos_2insert(size_t start, VALUE_TYPE t)const
  {
    if (length_ ==0)
      return -1;
    
    if (!AUTO_SORT)
      return length_-1;

    return binary_search(t, start, length_-1);
  }

protected:
   /**
     Swap two elements.
  **/
  void swap_(VALUE_TYPE* a, VALUE_TYPE* b)
  {
    VALUE_TYPE temp;
    temp = *a;
    *a = *b;
    *b = temp;
  }

  /**
     When quick sort, it return the index of media element.
   **/
  uint32_t findMedianIndex_(VALUE_TYPE* array, uint32_t left, uint32_t right)
  {
    return (left+right)/2;
    
//     VALUE_TYPE min = array[left];
//     VALUE_TYPE max = array[left];
//     uint32_t shift = 5;

//     if (right-left<2*shift)
//       return (left+right)/2;
    
//     for (uint32_t i=left+shift; i<=right; i+=shift)
//     {
//       if (array[i]<min)
//       {
//         min = array[i];
//         continue;
//       }

//       if (array[i] > max)
//       {
//         max = array[i];
//         continue;
//       }
//     }

//     VALUE_TYPE average = (min + max )/2;
//     uint32_t idx = left;
//     VALUE_TYPE minGap = average>array[idx]? average-array[idx] : array[idx]-average;
    
//     for (uint32_t i=left+shift; i<=right; i+=shift)
//     {
//       VALUE_TYPE gap = average>array[i]? average-array[i] : array[i]-average;
//       if (gap<minGap)
//       {
//         minGap = gap;
//         idx = i;
//       }
//     }

//     return idx;
    
  }
  
//   /**
//      Partition the array into two halves and return the index about which the array is partitioned.
//   **/
//   uint32_t partition_(VALUE_TYPE* array, uint32_t left, uint32_t right)
//   {
//     IASSERT(right<length_);
//     IASSERT(left<=right);
    
//     uint32_t pivotIndex = findMedianIndex_(array, left, right);
//     uint32_t index = left;
    
//     VALUE_TYPE pivotValue = array[pivotIndex];
 
//     swap_(&array[pivotIndex], &array[right]);

//     uint32_t i;
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

  /**
     A recursive function applying quick sort.
   **/
  void quickSort_(VALUE_TYPE* array, uint32_t left, uint32_t right)
  {
    int i = left, j = right;
    VALUE_TYPE tmp;
    VALUE_TYPE pivot = array[(left + right) / 2];
    
    /* partition */
    while (i <= j) {
      
      while (i < (int)length_ && array[i] < pivot)
        i++;
      while (j>=0 && array[j] > pivot )
        j--;

      if (i <= j) {
        tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
        i++;
        j--;
      }
    };

    //IASSERT(i-1==j);
    /* recursion */
    if ((int)left < j)
      quickSort_(array, left, j);

    if (i < (int)right)
      quickSort_(array, i, right);

    //     if (left == right)
//       return;
    
//     IASSERT(left < right);

//     IASSERT(right < length_);
    
//     if(right-left<=1)
//     {
//       if (array[left]>array[right])
//         swap_(&array[left], &array[right]);
//       return;
//     }
    
//     uint32_t idx = partition_(array, left, right);
    
//     if(idx>0 && left<idx-1)
//       quickSort_(array, left, idx - 1);
//     if (idx+1<length_ && idx+1<right)
//       quickSort_(array, idx + 1, right);
  }

public:
  
  inline explicit DynArray()
    :p_(NULL), length_(0), max_size_(0)
  {
  }

  /**
     @brief it can be initialized with std::vector
   */
  inline explicit DynArray(const std::vector<VALUE_TYPE>& v)
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
  inline void assign(const std::vector<VALUE_TYPE>& v)
  {
    if (v.size()==0)
    {
      clear();
      return;
    }

    derefer();
    
    new_one(v.size());
    memcpy(ARRAY(p_), v.data(), sizeof(VALUE_TYPE)*v.size());
  }

  /**
     @brief initialized with a buffer
   */
  inline void assign(const char* p, size_t len)
  {
    if (!len)
    {
      clear();
      return;
    }

    derefer();
    
    new_one(len/sizeof(VALUE_TYPE));
    memcpy(ARRAY(p_), p, len);
  }

  inline void assign(const SelfT& other)
  {
    //IASSERT(this!=&other);
    
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
    memcpy(ARRAY(p_), ARRAY(other.p_), sizeof(VALUE_TYPE)*other.length());
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
  
  inline SelfT& operator = (const std::vector<VALUE_TYPE>& other)
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

    memcpy(&ARRAY(p_)[length_], ARRAY(other.p_), other.size());
    length_ += other.length();

    return *this;
  }

  /**
     @brief Append a std::vector at the end
   */
  SelfT& operator += (const std::vector<VALUE_TYPE>& other)
  {
    if (other.size()==0)
      return *this;

    if (is_refered())
      assign_self();

    if (max_size_ < length_+other.size())
      enlarge(length_+other.size());
    
    memcpy(&ARRAY(p_)[length_], other.data(), other.size()*sizeof(VALUE_TYPE));
    
    length_ += other.size();
    
    return *this;
  }

  /**
     @brief appended by a element.
   */
  inline SelfT& operator += (VALUE_TYPE t)
  {
    push_back(t);
    return *this;
  }

  inline bool operator == (const VALUE_TYPE* other)
  {
    for (size_t i=0; i<length_; i++)
      if (at(i)!= other[i])
        return false;

    return true;
  }

  /**
     @see inline SelfT& operator += (VALUE_TYPE t)
   */
  inline size_t push_back(VALUE_TYPE t)
  {
    if (is_refered())
      assign_self();
    
    size_t n = find_pos_2insert(t);
    insert(n, t);
    
    return n+1;
  }

  /**
     @brief push back n elements of another vector.
   */
  void push_back(const SelfT& other, size_t n = 0)
  {
    if (other.length()==0 || n>=other.length())
      return;
    
    if (is_refered())
      assign_self();

    
    if (max_size_ < length_+other.length()-n)
      enlarge(length_+other.length()-n);
    
    memcpy(&ARRAY(p_)[length_], other.data()+n, (other.length()-n)*sizeof(VALUE_TYPE));
    
    length_ += other.length()-n;
  }

  /**
     @brief remove the last element
   */
  inline VALUE_TYPE pop_back()
  {
    if (length_ ==0)
      return VALUE_TYPE();
    
    if (is_refered())
      assign_self();
    
    VALUE_TYPE r = at(length_-1);
    --length_;
    return r;
  }

  /**
     @brief it's equal to push_back(), but make sure memory has been
     reserved and no reference before calling.
   */
  inline bool add_tail(VALUE_TYPE t)
  {
    // if (max_size_==0 || max_size_ == length_)
//       enlarge((max_size_+1)<<APPEND_RATE);
    
    IASSERT(length_<max_size_);
    ARRAY(p_)[length_] = t;
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
    
    for (size_t i=t; i<length_-1; i++)
      ARRAY(p_)[i] = ARRAY(p_)[i+1];
    length_--;
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
    max_size_ = length_ = 0;
  }  

  /**
     @brief insert t at n.
   */
  inline void insert(size_t n, VALUE_TYPE t)
  {
    IASSERT(n < length_ || n == NOT_FOUND);
    if (is_refered())
      assign_self();

    IASSERT(max_size_>=length_);
    
    if (max_size_==0 || max_size_ == length_)
      enlarge((size_t)((max_size_+1)*1.2+.5));//<<APPEND_RATE);
    
    for (size_t i=length_; i>n+1; i--)
      ARRAY(p_)[i] = ARRAY(p_)[i-1];
    
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

//     const uint32_t SIZE = 1000000;
//     if (length_/SIZE <=2)
//       return quickSort_(ARRAY(p_), 0, length_-1);

//     const uint32_t LEN = length_%SIZE==0? length_/SIZE: length_/SIZE+1;
//     uint32_t* index = (uint32_t*)malloc(sizeof(uint32_t)*LEN);
//     for (uint32_t i=0; i<LEN; ++i)
//       index[i] = 0;
    
//     size_t i=0;
//     for (; i<length_; i+=SIZE)
//       quickSort_(ARRAY(p_), i, ((i+SIZE-1)>length_-1? length_-1: (i+SIZE-1)));;

//     char* p = (char*)malloc(length_*sizeof(VALUE_TYPE)+sizeof(ReferT));
//     i = 0;
//     uint32_t finished = 0;

//     while (finished != LEN)
//     {
//       uint32_t mini = 0;
//       VALUE_TYPE min;

//       uint32_t j = 0;
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

  size_t compressed_save(FILE* f, uint64_t addr = -1)
  {
    if (size()%sizeof(uint32_t)!=0)
      return save(f, addr);
    
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);

    size_t len = size();
    IASSERT(fwrite(&len, sizeof(size_t), 1, f)==1);
    len /= sizeof(uint32_t);
    
    uint32_t* arr = (uint32_t*)ARRAY(p_);
    size_t s = 0;
    uint8_t* buff = new uint8_t[size()*2];
    
    for (uint32_t i=0; i<len; ++i)
    {
      uint32_t ui = arr[i];
      while ((ui & ~0x7F) != 0)
      {
        buff[s++] = ((uint8_t)((ui & 0x7f) | 0x80));
        ui >>= 7;
      }
      buff[s++] = ( (uint8_t)ui );
    }
    
    IASSERT(fwrite(&s, sizeof(size_t), 1, f)==1);

    if (s > 0)
      IASSERT(fwrite(buff, s, 1, f)==1);

    delete buff;
    
    return s+sizeof(size_t)*2;
  }
  
  size_t compressed_load(FILE* f, uint64_t addr = -1)
  {
    clear();
    
    if (addr != (uint64_t)-1)
      fseek(f, addr, SEEK_SET);
    
    size_t s = 0;

    IASSERT(fread(&s, sizeof(size_t), 1, f)==1);

    size_t LEN = s/sizeof(uint32_t);
    max_size_ = length_ = s/sizeof(VALUE_TYPE);
    new_one(length_);

    if (s % sizeof(uint32_t)!=0)
    {
      if (s > 0)
        IASSERT(fread(ARRAY(p_), s, 1, f)==1);
      return s;
    }

    IASSERT(fread(&s, sizeof(size_t), 1, f)==1);
    if (s == 0)
      return s;

    uint32_t* arr = (uint32_t*)ARRAY(p_);
    size_t len = 0;
    uint8_t* buff = new uint8_t[s];
    IASSERT(fread(buff, s, 1, f)==1);
    for (size_t c = 0;c<s && len<LEN; ++len)
    {
      uint8_t b = buff[c++];
      uint32_t i = b & 0x7F;
      for (uint32_t shift = 7; (b & 0x80) != 0; shift += 7)
      {
        b = buff[c++];
        i |= (b & 0x7FL) << shift;
      }
      arr[len] = i;
    }

    delete buff;
    return s+2*sizeof(size_t);
  }

  /**
    @brief it load from a memory buffer.
   */
  size_t load(const char* mem)
  {
    if (is_refered())
      assign_self();
        
    size_t s = *(size_t*)mem;
    if ( 0 == s)
      return sizeof (size_t);

    max_size_ = length_ = s/sizeof(VALUE_TYPE);

    new_one(length_);

    memcpy(ARRAY(p_), mem+sizeof(size_t), s);

    return s+sizeof(size_t);
  }

  /**
    @brief it save from a memory buffer.
   */
  size_t save(char* mem)const
  {
    *(size_t*)mem = size();
    memcpy( mem+sizeof(size_t), ARRAY(p_), size() );

    return size()+sizeof(size_t);
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

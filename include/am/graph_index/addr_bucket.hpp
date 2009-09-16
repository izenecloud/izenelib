#ifndef ADDR_BUCKET_HPP
#define ADDR_BUCKET_HPP

#include<types.h>
#include <ostream>
#include <iostream>
#include <assert.h>
#include "dyn_array.hpp"
#include <string>


NS_IZENELIB_AM_BEGIN

template<
  typename VALUE_TYPE = uint64_t,
  uint32_t BUF_SIZE = 1000
  >
class FileDataBucket
{
  typedef DynArray<VALUE_TYPE> array_t;

  array_t buf_;
  FILE* f_;
  uint64_t num_;
  std::string filenm_;
  uint64_t fetch_i_;

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
  
  /**
     Partition the array into two halves and return the index about which the array is partitioned.
  **/
  uint32_t partition_(VALUE_TYPE* array, uint32_t left, uint32_t right)
  {
    uint32_t pivotIndex = findMedianIndex_(array, left, right), index = left, i;
    VALUE_TYPE pivotValue = array[pivotIndex];
 
    swap_(&array[pivotIndex], &array[right]);
    
    for(i = left; i < right; i++)
    {
      if(array[i] < pivotValue)
      {
        swap_(&array[i], &array[index]);
        index += 1;
      }
    }
    swap_(&array[right], &array[index]);
 
    return index;
  }

  /**
     A recursive function applying quick sort.
   **/
  void quickSort_(VALUE_TYPE* array, uint32_t left, uint32_t right)
  {
    if(right-left<=1)
    {
      if (array[left]>array[right])
        swap_(&array[left], &array[right]);
      return;
    }
    
    uint32_t idx = partition_(array, left, right);
    
    if(idx>0 && left<idx-1)
      quickSort_(array, left, idx - 1);
    if (idx+1<right)
      quickSort_(array, idx + 1, right);
  }
  
public:
  FileDataBucket(const char* nm)
  {
    filenm_ = nm;
    buf_.reserve(BUF_SIZE/sizeof(VALUE_TYPE));
    num_ = 0;
  }

  void ready4add()
  {
    f_ = fopen(filenm_.c_str(), "r+");
    if (f_ == NULL)
    {
      f_ = fopen(filenm_.c_str(), "w+");
      assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    }
    else
      assert(fread(&num_, sizeof(uint64_t), 1, f_)==1);

    fseek(f_, 0, SEEK_END);

    buf_.reset();
  }

  void push_back(const VALUE_TYPE& v)
  {
    if (buf_.length()>=BUF_SIZE/sizeof(VALUE_TYPE))
    {
      buf_.save(f_);
      buf_.reset();
    }

    buf_.push_back(v);
    ++num_;
  }

  void flush()
  {
    buf_.save(f_);

    fseek(f_, 0, SEEK_SET);
    assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    
    fflush(f_);
    fclose(f_);

    buf_.reset();
  }
  
  void ready4fetch()
  {
    f_ = fopen(filenm_.c_str(), "r+");

    if (f_ == NULL)
    {
      std::cout<<"can't open file: "<<filenm_<<std::endl;
      return;
    }
    else
      assert(fread(&num_, sizeof(uint64_t), 1, f_)==1);

    fetch_i_ = 0;

    buf_.load(f_);
  }

  uint64_t num()const
  {
    return num_;
  }
  
  VALUE_TYPE next()
  {
    if (fetch_i_ >= buf_.length())
    {
      buf_.load(f_);
      fetch_i_ = 0;
      return buf_.at(0);
    }

    return buf_.at(fetch_i_++);
  }
  

  void sort()
  {
    if (num_<=1)
      return;

    array_t array;
    array.reserve(num_);
    
    ready4fetch();
    array += buf_;

    uint32_t s = BUF_SIZE/sizeof(VALUE_TYPE);
    s = num_%s==0? num_/s: num_/s+1;
    for (uint32_t i=1; i<s; ++i)
    {      
      buf_.load(f_);
      array += buf_;
    }

    fclose(f_);

    //std::cout<<array.length()<<"++++\n";
    array.sort();
    //std::cout<<array.length()<<"----\n";
    
    //quickSort_(array.data(), 0, array.length()-1);

    //std::cout<<array<<std::endl;
    
    num_ = 0;
    f_ = fopen(filenm_.c_str(), "w+");
    assert(fwrite(&num_, sizeof(uint64_t), 1, f_)==1);
    fclose(f_);

    ready4add();
    
    for (uint32_t i=0; i<array.length(); ++i)
      push_back(array.at(i));

    flush();
  }

  void dump()
  {
    fclose(f_);
    remove(filenm_.c_str());
  }
  
}
  ;

NS_IZENELIB_AM_END

#endif

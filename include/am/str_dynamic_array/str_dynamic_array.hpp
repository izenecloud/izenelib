/**
   @file str_dynamic_array.hpp
   @author kevin hu
   @date 2009.11.25
 */
#ifndef STR_DYNAMIC_ARRAY
#define STR_DYNAMIC_ARRAY

#include <time.h>
#include <string>
#include <types.h>
#include <iostream>
#include <ostream>
#include <iterator>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
   @class DynamicStrArray
   @brief A dynamic string array.
 */
class DynamicStrArray
{
  enum 
    {
      BUCKET_SIZE = 64,
      UINT64_SIZE = sizeof(uint64_t),
      UINT32_SIZE = sizeof(uint32_t),
      CHAR_SIZE = sizeof(char)
    };
  
    
public:
  DynamicStrArray()
  {
    buff_ = new char [BUCKET_SIZE];
    idxs_ = new uint64_t [BUCKET_SIZE];
    
    max_size_ = BUCKET_SIZE;
    count_ = 0;
    tail_ = 0;
    pos_ = (uint64_t)-1;
    
    buf_buf_ = NULL;
    buf_buf_size_ = 0;
    idxs_buf_ = NULL;
    idxs_buf_size_ = 0;
  }

  ~DynamicStrArray()
  {
    delete buff_;
    delete idxs_;
  }
  
  uint64_t size()const 
  {
    return count_;
  }

  uint64_t max_size()const
  {
    return max_size_;
  }

  void resize(uint64_t s)
  {
  }

  bool empty() const
  {
    return count_ ==0;
  }
  
  string const& operator [] (uint64_t i)
  {
    if (i>= count_)
    {
      cout<<"DynammicStrArray::operator [] exceed boundary!\n";
    }

    ret_.assign(buff_ + idxs_[i], i<count_-1? idxs_[i+1]-idxs_[i]: tail_ - idxs_[i]);
    return ret_;
  }

  string const& at (uint64_t i)
  {    
    if (i>= count_)
    {
      cout<<"DynammicStrArray::at() exceed boundary!\n";
    }

    ret_.assign(buff_ + idxs_[i], i<count_-1? idxs_[i+1]-idxs_[i]: tail_ - idxs_[i]);
    return ret_;
  }

  string const& front()
  {
    ret_.assign(buff_, count_>1? idxs_[1]: tail_);
    return ret_;
  }

  string const& back()
  {
    ret_.assign(buff_ + idxs_[count_-1], tail_ - idxs_[count_-1]);
    return ret_;
  }

  void push_back(const string& str)
  {
    size_t s = str.length()+1;

    if (s >= BUCKET_SIZE)
    {
      max_size_ += (s/BUCKET_SIZE + 1)*BUCKET_SIZE;
      buff_ = (char*)realloc(buff_, max_size_ );
    }

    else if (s > max_size_ - tail_)
    {
      max_size_ += BUCKET_SIZE;
      buff_ = (char*)realloc(buff_, max_size_ );
    }

    if (count_ > 0 && count_%BUCKET_SIZE ==0 )
    {
      idxs_ = (uint64_t*)realloc((void*)idxs_, (count_+ BUCKET_SIZE)*UINT64_SIZE);
    }

    idxs_[count_] = tail_;
    
    memcpy(buff_+tail_, str.c_str(), s);
    tail_ += s;
    count_++;

    return;
  }

  void pop_back ( )
  {
    tail_ = idxs_[count_-1];
    count_--;
  }
  
  void clear()
  {
    delete buff_;
    delete idxs_;
    
    buff_ = new char [BUCKET_SIZE];
    idxs_ = new uint64_t [BUCKET_SIZE];
    max_size_ = BUCKET_SIZE;
    count_ = 0;
    tail_ = 0;
  }

friend ostream& operator << (ostream& os, const izenelib::am::DynamicStrArray& dy)
  {
    os << "\n[count_, tail_, max_size_]: "<<dy.count_<<","<<dy.tail_<<","<<dy.max_size_<<endl;    
    
    return os;
  }

  class const_iterator : public iterator<input_iterator_tag, string>
  {
    string iref_;
    DynamicStrArray* p_;
    
  public:

    uint64_t i_;
    
    const_iterator(DynamicStrArray* p, uint64_t i = 0)
    {
      p_ = p;
      i_ = i;

      if (i>=p_->count_)
        return;
      
      iref_.assign(p->buff_ + p_->idxs_[i], i<p_->count_-1? p_->idxs_[1+i]-p_->idxs_[i]: p_->tail_ - p_->idxs_[i]);
    }
    
    const_iterator(const const_iterator& it)
    {
      p_ = it.p_;
      
      i_ = it.i_;
      iref_ = it.iref_;
    }

    const_iterator& operator++()
    {
      i_++;

      if (i_>=p_->count_)
        return *this;
      
      iref_.assign(p_->buff_+p_->idxs_[i_] , i_<p_->count_-1? p_->idxs_[1+i_]- p_->idxs_[i_]: p_->tail_ - p_->idxs_[i_]);
      
      return *this;
    }
    
    const_iterator& operator++ (int)
    {
      
      i_++;

      if (i_>=p_->count_)
        return *this;
      
      iref_.assign(p_->buff_+p_->idxs_[i_] , i_<p_->count_-1? p_->idxs_[1+i_]-p_->idxs_[i_]: p_->tail_ - p_->idxs_[i_]);
      
      return *this;
    }

    bool operator==(const const_iterator& rhs)
    {
      return p_==rhs.p_ && i_==rhs.i_;
    }
    
    bool operator!=(const const_iterator& rhs)
    {
      return p_!=rhs.p_ || i_!=rhs.i_;
    }
    
    string const& operator*() {return iref_;}
  };


  const_iterator begin()
  {
    return const_iterator(this);
  }

  const_iterator end()
  {
    return const_iterator(this, count_);
  }

  const_iterator find(const string& str)
  {
    size_t len = str.length()+1;
    
    for (uint64_t i =0;i <count_-1; i++)
    {
      uint64_t s = idxs_[i+1]-idxs_[i];
      if (len != s)
        continue;

      bool f = true;
      for (uint64_t j=0; j<len; j++)
        if (*(buff_ + idxs_[i] + j)!= str[j])
        {
          f = false;
          break;
        }

      if (f)
        return const_iterator(this, i);
      
    }

    uint64_t s = tail_ - idxs_[count_ -1];
    if (len==s)
    {      
      bool f = true;
      for (uint64_t j=0; j<len; j++)
        if (*(buff_ + idxs_[count_-1] + j)!= str[j])
        {
          f = false;
          break;
        }

      if (f)
        return const_iterator(this, count_-1);
    }

    return const_iterator(this, count_);
  }

  const_iterator insert (const_iterator pos, const string& str )
  {
    size_t s = str.length()+1;

    if (s >= BUCKET_SIZE)
    {
      max_size_ += (s/BUCKET_SIZE + 1)*BUCKET_SIZE;
      buff_ = (char*)realloc(buff_, max_size_ );
    }

    else if (s > max_size_ - tail_)
    {
      max_size_ += BUCKET_SIZE;
      buff_ = (char*)realloc(buff_, max_size_ );
    }

    if (count_ > 0 && count_%BUCKET_SIZE ==0 )
    {
      idxs_ = (uint64_t*)realloc((void*)idxs_, (count_+ BUCKET_SIZE)*UINT64_SIZE);
    }

    if (buf_buf_size_<tail_ - idxs_[pos.i_] || buf_buf_==NULL)
    {
      if (buf_buf_!=NULL)
        delete buf_buf_;
      buf_buf_ = new char[(uint64_t)((tail_ - idxs_[pos.i_])*1.2)];
      buf_buf_size_ = (uint64_t)((tail_ - idxs_[pos.i_])*1.2);
    }
    
    memcpy(buf_buf_, buff_+idxs_[pos.i_], tail_ - idxs_[pos.i_]);
    memcpy(buff_+idxs_[pos.i_]+s, buf_buf_, tail_ - idxs_[pos.i_]);
    memcpy(buff_+idxs_[pos.i_], str.c_str(), s);

    if (idxs_buf_size_<count_ - pos.i_ || idxs_buf_==NULL)
    {
      if (idxs_buf_!=NULL)
        delete idxs_buf_;
      idxs_buf_ = new uint64_t[(uint64_t)((count_ - pos.i_)*1.2)];
      idxs_buf_size_ = (uint64_t)((count_ - pos.i_)*1.2);
    }
    memcpy(idxs_buf_, (char*)(idxs_ + pos.i_), (count_ - pos.i_)*UINT64_SIZE);
    memcpy((char*)(idxs_ + pos.i_+1), idxs_buf_, (count_ - pos.i_)*UINT64_SIZE);
    
    tail_ += s;
    count_++;
    for (uint64_t i=pos.i_+1; i<count_;i++)
      idxs_[i] += s;
    
    return const_iterator(this, pos.i_);
  }

  const_iterator erase (const const_iterator& pos )
  {
    if (pos.i_>=count_)
      return end();

    if (pos.i_==count_-1)
    {
      pop_back();
      return const_iterator(this, count_-1);
    }
    
    if (buf_buf_size_<tail_ - idxs_[pos.i_+1] || buf_buf_==NULL)
    {
      if (buf_buf_!=NULL)
        delete buf_buf_;
      buf_buf_ = new char[(uint64_t)((tail_ - idxs_[pos.i_+1])*1.2)];
      buf_buf_size_ = (uint64_t)((tail_ - idxs_[pos.i_+1])*1.2);
    }
    
    memcpy(buf_buf_, buff_+idxs_[pos.i_+1], tail_ - idxs_[pos.i_+1]);
    memcpy(buff_+idxs_[pos.i_], buf_buf_, tail_ - idxs_[pos.i_+1]);

    uint64_t s = idxs_[pos.i_+1]-idxs_[pos.i_];
    if (idxs_buf_size_<count_ - pos.i_-1 || idxs_buf_==NULL)
    {
      if (idxs_buf_!=NULL)
        delete idxs_buf_;
      idxs_buf_ = new uint64_t[(uint64_t)((count_ - pos.i_-1)*1.2)];
      idxs_buf_size_ = (uint64_t)((count_ - pos.i_-1)*1.2);
    }
    memcpy(idxs_buf_, (char*)(idxs_ + pos.i_+1), (count_ - pos.i_-1)*UINT64_SIZE);
    memcpy((char*)(idxs_ + pos.i_), idxs_buf_, (count_ - pos.i_-1)*UINT64_SIZE);
    
    tail_ -= s;
    count_--;
    for (uint64_t i=pos.i_; i<count_;i++)
      idxs_[i] -= s;

    return const_iterator(this, pos.i_);

  }

  /**
   *first, last
    Iterators specifying a range within the vector to be removed: [first,last).
    i.e., the range includes all the elements between first and last, including
    the element pointed by first but not the one pointed by last.
  **/
  const_iterator erase (const const_iterator& first, const const_iterator& last )
  {
    if (first.i_>=last.i_)
      return end();
    
    if (last.i_>=count_)
    {
      tail_ = idxs_[first.i_];
      count_ -= last.i_ - first.i_;
      return const_iterator(this, count_-1);
    }

    if (buf_buf_size_<tail_ - idxs_[last.i_] || buf_buf_==NULL)
    {
      if (buf_buf_!=NULL)
        delete buf_buf_;
      buf_buf_ = new char[(uint64_t)((tail_ - idxs_[last.i_])*1.2)];
      buf_buf_size_ = (uint64_t)((tail_ - idxs_[last.i_])*1.2);
    }
    
    memcpy(buf_buf_, buff_+idxs_[last.i_], tail_ - idxs_[last.i_]);
    memcpy(buff_+idxs_[first.i_], buf_buf_, tail_ - idxs_[last.i_]);

    uint64_t s = idxs_[last.i_] - idxs_[first.i_];
    if (idxs_buf_size_ < count_ - last.i_ || idxs_buf_==NULL)
    {
      if (idxs_buf_!=NULL)
        delete idxs_buf_;
      idxs_buf_ = new uint64_t[(uint64_t)((count_ - last.i_)*1.2)];
      idxs_buf_size_ = (uint64_t)((count_ - last.i_-1)*1.2);
    }
    memcpy(idxs_buf_, (char*)(idxs_ + last.i_), (count_ - last.i_)*UINT64_SIZE);
    memcpy((char*)(idxs_ + first.i_), idxs_buf_, (count_ - last.i_)*UINT64_SIZE);
    
    tail_ -= s;
    count_ -= last.i_ - first.i_;
    
    for (uint64_t i=first.i_; i<count_;i++)
      idxs_[i] -= s;

    return const_iterator(this, first.i_);

  }

  struct unit
  {
    uint64_t key_;
    uint64_t i_;
    DynamicStrArray* p_;
      
    inline unit (DynamicStrArray* p, uint64_t key, uint64_t i)
    {
      key_ = key;
      i_ = i;
      p_ = p;
    }
      
    inline unit(DynamicStrArray* p, uint64_t i)
    {
      p_ = p;
      char * start = p_->buff_ + p_->idxs_[i];
        
      uint64_t len = i<p_->count_-1 ? p_->idxs_[i+1]-p_->idxs_[i]: p_->tail_ - p_->idxs_[i];
      i_ = i;
      key_ = 0;

      for(uint64_t i=0; i<UINT64_SIZE-1; i++)
      {
        if (i<len)
          key_ += (uint64_t)(*(start+i));

        key_ <<= 8;
      }
    }

    inline unit()
    {
      i_ = key_ = -1;
    }

    bool operator > (const unit& u)
    {
      if (key_ != u.key_)
        return key_>u.key_;

      uint64_t len = i_<p_->count_-1? p_->idxs_[i_+1]-p_->idxs_[i_]: p_->tail_ - p_->idxs_[i_];
      uint64_t u_len = u.i_<p_->count_-1? p_->idxs_[u.i_+1]-p_->idxs_[u.i_]: p_->tail_ - p_->idxs_[u.i_];

      if (len < UINT64_SIZE)
        return false;

      uint64_t j=UINT64_SIZE;
      for (; j<len && j<u_len; j++)
      {
        if (*(p_->buff_+p_->idxs_[i_]+j) > *(p_->buff_+p_->idxs_[u.i_]+j) )
          return true;
        if (*(p_->buff_+p_->idxs_[i_]+j) < *(p_->buff_+p_->idxs_[u.i_]+j) )
          return false;
      }

      if (len>u_len)
        return true;
        
      return false;
    }
      
    bool operator < (const unit& u)
    {
      if (key_ != u.key_)
        return key_<u.key_;

      uint64_t len = i_<p_->count_-1? p_->idxs_[i_+1]-p_->idxs_[i_]: p_->tail_ - p_->idxs_[i_];
      uint64_t u_len = u.i_<p_->count_-1? p_->idxs_[u.i_+1]-p_->idxs_[u.i_]: p_->tail_ - p_->idxs_[u.i_];

      if (len < UINT64_SIZE)
        return false;

      uint64_t j=UINT64_SIZE;
      for (; j<len && j<u_len; j++)
      {
        if (*(p_->buff_+p_->idxs_[i_]+j) < *(p_->buff_+p_->idxs_[u.i_]+j) )
          return true;
        if (*(p_->buff_+p_->idxs_[i_]+j) > *(p_->buff_+p_->idxs_[u.i_]+j) )
          return false;
      }

      if (len<u_len)
        return true;
        
      return false;
    }
  };

  void sort()
  {
    unit* us = new unit[count_];
    
    for (uint64_t i=0; i<count_; i++)
    {
      us[i] = unit(this, i);
    }
    
    
    quickSort(us, 0, count_-1);
    
    char* buf = new char[max_size_];
    uint64_t* idxs = new uint64_t[count_%BUCKET_SIZE==0? count_: (count_/BUCKET_SIZE+1)*BUCKET_SIZE];
    idxs[0] = 0;
    
    for (uint64_t i=0; i<count_; i++)
    {
      uint64_t k = us[i].i_;
      
      uint64_t len = k<count_-1? idxs_[k+1]-idxs_[k]: tail_ - idxs_[k];
      memcpy(buf + idxs[i], buff_ + idxs_[k], len);
      
      if (i < count_-1)
        idxs[i+1] = idxs[i]+len;
    }

    delete buff_;
    delete idxs_;
    
    buff_ = buf;
    idxs_ = idxs;
    //cout<<buff_ + idxs_[2]<<endl;
    
    delete us;
  }
  
  
friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version)  const
  {
    ar & count_;
    ar & tail_;
    ar & max_size_;
    
    ar.save_binary(buff_, max_size_);
    ar.save_binary(idxs_, (count_%BUCKET_SIZE==0? count_: (count_/BUCKET_SIZE+1)*BUCKET_SIZE)*UINT64_SIZE);
  }

    
  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
    ar & count_;
    ar & tail_;
    ar & max_size_;

    delete buff_;
    buff_ = new char[max_size_];
    uint64_t s = (count_%BUCKET_SIZE==0? count_: (count_/BUCKET_SIZE+1)*BUCKET_SIZE);
    delete idxs_;
    idxs_ = new uint64_t[s];
    
    ar.load_binary(buff_, max_size_);
    ar.load_binary(idxs_, s*UINT64_SIZE);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
  
  /**
     A recursive function applying quick sort.
   **/
  void quickSort(unit* array, uint64_t left, uint64_t right)
  {
    if(right-left<=1)
    {
      if (array[left]>array[right])
        swap(&array[left], &array[right]);
      return;
    }
    
    uint64_t idx = partition(array, left, right);
    
    if(idx-1!=(uint64_t)-1 && left<idx-1)
      quickSort(array, left, idx - 1);
    
    if (idx!=(uint64_t)-1 && idx+1<right)
      quickSort(array, idx + 1, right);
  }

  
  /**
     Swap two elements.
  **/
  void swap(unit* a, unit* b)
  {
    unit temp;
    temp = *a;
    *a = *b;
    *b = temp;
  }

    
  /**
     Partition the array into two halves and return the index about which the array is partitioned.
  **/
  uint64_t partition(unit* array, uint64_t left, uint64_t right)
  {
    uint64_t pivotIndex = (left+right)/2, index = left, i;//findMedianIndex(array, left, right), index = left, i;
    unit pivotValue = array[pivotIndex];
 
    swap(&array[pivotIndex], &array[right]);
    
    for(i = left; i < right; i++)
    {
      if(array[i] < pivotValue)
      {
        swap(&array[i], &array[index]);
        index += 1;
      }
    }
    swap(&array[right], &array[index]);
 
    return index;
  }


  
protected:
  char* buff_;
  uint64_t* idxs_;
  
  uint64_t count_;
  uint64_t tail_;
  uint64_t max_size_;


  
protected:
  string ret_;
  uint64_t pos_;
  char* buf_buf_;
  uint64_t buf_buf_size_;
  uint64_t* idxs_buf_;
  uint64_t idxs_buf_size_;
}
  ;

NS_IZENELIB_AM_END

#endif

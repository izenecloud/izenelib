#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <stdio.h>
#include <string>
#include <types.h>
#include <algorithm>
#include "alphabet.h"
#include <vector>

using namespace std;


template<
  class DataType = string,
  uint32_t BUCKET_SIZE = 8192,//byte
  uint8_t SPLIT_RATIO = 75,
  char* ALPHABET = a2z,
  uint8_t ALPHABET_SIZE = a2z_size 
  >
class Bucket
{
protected:

  #define DISK_STR_BUF_SIZE BUCKET_SIZE - sizeof(uint32_t)*2 -2*sizeof(uint8_t)
#define ONE_STRING_SIZE(STR) (STR).length()+sizeof(uint32_t)+sizeof(uint64_t)

  struct _disk_bucket_
  {
    uint8_t from_;
    uint8_t to_;
    uint32_t count_;
    uint32_t size_;
    char strBuf[BUCKET_SIZE - sizeof(uint32_t)*2 -2*sizeof(uint8_t)];
  }
    ;

  class _string_ptr_
  {
  public:
    string* p_;
    uint64_t contentAddr_;
    
    _string_ptr_(string* p, uint64_t addr)
      :p_(p),contentAddr_(addr)
    {
    }

    
    bool operator == (const _string_ptr_& p)
    {
      return p_->compare(*p.p_)==0;
    }

    bool operator >  (const _string_ptr_& p)
    {
      return p_->compare(*p.p_)>0;
    }

    bool operator <  (const  _string_ptr_& p)
    {
      return p_->compare(*p.p_)<0;
    }

    bool operator == (const string* p)
    {
      return p_->compare(*p)==0;
    }

    bool operator >  (const string* p)
    {
      return p_->compare(*p)>0;
    }

    bool operator <  (const string* p)
    {
      return p_->compare(*p)<0;
    }
    
  }
    ;
  typedef typename vector<_string_ptr_>::iterator str_ptr_it;
  typedef typename vector<_string_ptr_>::const_iterator const_str_ptr_it;
  
  class _string_group_
  {
  public:
    ~ _string_group_()
    {
    }
    
    uint8_t firstChar_;
    vector<_string_ptr_> strPtrs_;

    bool operator == (const _string_group_& sg)
    {
      return firstChar_==sg.firstChar_;
    }

    bool operator > (const _string_group_& sg)
    {
      return firstChar_ > sg.firstChar_;
    }

    bool operator < (const _string_group_& sg)
    {
      return firstChar_ < sg.firstChar_;
    }

    bool operator >= (const _string_group_& sg)
    {
      return firstChar_ >= sg.firstChar_;
    }

    bool operator <= (const _string_group_& sg)
    {
      return firstChar_ <= sg.firstChar_;
    }
    //////////////////////////
    bool operator == (const string& str)
    {
      return firstChar_==str[0];
    }

    bool operator > (const string& str)
    {
      return firstChar_ > str[0];
    }

    bool operator < (const string& str)
    {
      return firstChar_ < str[0];
    }

    bool operator >= (const string& str)
    {
      return firstChar_ >= str[0];
    }

    bool operator <= (const string& str)
    {
      return firstChar_ <= str[0];
    }

    size_t addString(const string& str, uint64_t addr)
    {
      _string_ptr_ p(new string(str.length()>1?str.substr(1):""), addr);
      str_ptr_it it = lower_bound(strPtrs_.begin(), strPtrs_.end(), p);
      if (it!=strPtrs_.end() && (*it)==p)
        return strPtrs_.size();

      //cout<<*p.p_<<"----------\n";
      strPtrs_.insert(it, p);
      return strPtrs_.size();
    }
    
  }
    ;

  typedef typename vector<struct _string_group_>::iterator str_group_it;
  typedef typename vector<struct _string_group_>::const_iterator const_str_group_it;
  struct _bucket_
  {
    uint8_t from_;
    uint8_t to_;
    uint32_t count_;
    uint32_t size_;
    bool dirty_;
    vector<_string_group_> strGroup_;
    uint64_t diskPos_;

    inline _bucket_(uint8_t from, uint8_t to)
    {
      /*
        if(from>to)
        throw exception;
      */
      from_ = from;
      to_ = to;
      count_ = 0;
      size_ = sizeof(uint32_t)*2+2*sizeof(uint8_t);//byte
      dirty_ = true;
      diskPos_ = -1;
    }
    
    inline _bucket_(struct _disk_bucket_* disk, uint64_t addr )
    {
      from_ = disk->from_;
      to_ = disk->to_;
      count_ = disk->count_;
      diskPos_ = addr;
      dirty_ = false;
      size_ = disk->size_;

      uint32_t t=0;
      uint8_t first = -1;
      for (uint32_t i =0; i<count_; i++)
      {
        uint32_t len = *(uint32_t*)(disk->strBuf+t);
        t += sizeof (uint32_t);
        if (t > DISK_STR_BUF_SIZE)
        {
          //THROW exception
          return;
        }
        
        string* s = new string(disk->strBuf+t+1, len-1);
        uint8_t firstCh = *(disk->strBuf+t);
        t += len;
        if (t > DISK_STR_BUF_SIZE)
        {
          //THROW exception
          return;
        }
        uint64_t cont = *(uint64_t*)(disk->strBuf+t);
        t += sizeof(uint64_t);
        if (t > DISK_STR_BUF_SIZE)
        {
          //THROW exception
          return;
        }
        
        if (first!=(*s)[0])
        {
          _string_group_ sg;
          sg.firstChar_ = firstCh;
          sg.strPtrs_.push_back(_string_ptr_(s, cont));
          first = firstCh;
          strGroup_.push_back(sg);
        }
        else
        {
          strGroup_.back().strPtrs_.push_back(_string_ptr_(s, cont));
        }
      }
      
    }
    
  }
    ;
  
public:
  typedef Bucket< DataType,BUCKET_SIZE,SPLIT_RATIO,ALPHABET, ALPHABET_SIZE > SelfType;
  Bucket(FILE* f)
    :f_(f),pBucket_(NULL)
  {
    pBucket_ = new _bucket_(ALPHABET[0], ALPHABET[ALPHABET_SIZE-1]);
  }

  ~Bucket()
  {
    for (str_group_it i=pBucket_->strGroup_.begin(); i!=pBucket_->strGroup_.end();i++)
    {
      for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end();j++)
        if ((*j).p_!=NULL)
          delete (*j).p_;
    }
    delete pBucket_;
  }
  
  bool load(uint64_t addr)
  {
    struct _disk_bucket_ b;
    if(fread(f_, &b, sizeof(struct _disk_bucket_))!=1)
      return false;

    if (pBucket_!=NULL)
      delete pBucket_;
    
    pBucket_ = new struct _bucket_(&b, addr);
       
    return true;
  }

  uint32_t bucket2disk(struct _bucket_* b, struct _disk_bucket_* d)
  {
    d->from_ = b->from_;
    d->to_ = b->to_;
    d->count_ = b->count_;

    uint32_t t=0;
    
    for (str_group_it i=b->strGroup_.begin(); i!=b->strGroup_.end();i++)
    {
      for (str_ptr_it j=(*i)->strPtrs_.begin();j!=(*i)->strPtrs_.end(); j++)
      {
        *(uint32_t*)(d->strBuf+t) = (*j).p_->length()+1;
        t += sizeof(uint32_t);
        *(d->strBuf+t) = (*i).firstCh_;
        t++;
        memcpy(d->strBuf+t, (*j).p_->c_str(), (*j).p_->length());
        t += (*j).p_->length();
        *(uint64_t*)(d->strBuf+t) = (*j).contentAddr_;
        t += sizeof(uint64_t);
      }
    }

    return t;
    
  }

  bool update2disk()
  {
    if (pBucket_==NULL)
      return false;

    if (pBucket_->dirty)
    {
      
      if (pBucket_->diskPos_ == (uint64_t)-1)
      {
        return ((add2disk()==(uint64_t)-1)? false: true);
      }

      struct _disk_bucket_ b;
      fseek(f_, pBucket_->diskPos_, SEEK_SET);
      bucket2disk(pBucket_, &b);
      return fwrite(&b, sizeof(struct _disk_bucket_), 1, f_)==1;
    }

    return true;
  }

  uint64_t add2disk()
  {
    if (pBucket_==NULL)
      return (uint64_t)-1;

    pBucket_ = NULL;
    
    fseek(f_, 0, SEEK_END);
    uint64_t end = ftell(f_);
    
    if (end%2==1)
    {//bucket only be stored at even address, which distinguish from alphabet node
      fseek(f_, 1, SEEK_END);
      end++;
    }
    
    
    struct _disk_bucket_ b;
    bucket2disk(pBucket_, &b);
    if (fwrite( &b, sizeof(struct _disk_bucket_), 1, f_)!=1)
      return (uint64_t) -1;

    return end;
  }

  bool updateContent(const string& str, const DataType& data)
  {
    return true;
  }
  
  void getContentBy(const string& str, DataType& data)
  {
  }

friend ostream& operator << ( ostream& os, const SelfType& node)
  {
    cout<<endl;
    cout<<"-----------------------------\n";
    cout<<"Range: "<<node.pBucket_->from_<<"--"<<node.pBucket_->to_<<endl;
    cout<<"Count: "<<node.pBucket_->count_<<"  Size: "<<node.pBucket_->size_<<endl;
    cout<<"Dirty: "<<node.pBucket_->dirty_<<"  Disk Pos: "<<node.pBucket_->diskPos_<<endl;
    
    for (str_group_it i=node.pBucket_->strGroup_.begin(); i!=node.pBucket_->strGroup_.end();i++)
    {
      cout<<"@"<<(*i).firstChar_<<" ........."<<endl;
      for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end();j++)
      {
        cout<<(*(*j).p_)<<" =>"<<(*j).contentAddr_<<endl;
      }
    }

    return os;
  }

  uint32_t addString(const string& str, uint64_t addr)
  {
    if (!canAddString(str))
      return pBucket_->size_;

    if (str.length()==0)
      return pBucket_->size_;

    if (str[0]<pBucket_->from_ || str[0]>pBucket_->to_)
      return pBucket_->size_;
    
    str_group_it t = lower_bound(pBucket_->strGroup_.begin(), pBucket_->strGroup_.end(), str);
    pBucket_->size_ += ONE_STRING_SIZE(str);
    if (t!=pBucket_->strGroup_.end())
    {
      if ((*t)==str)
      {
        if((*t).strPtrs_.size()<(*t).addString(str, addr))
          pBucket_->count_++;
      }
      else
      {
        _string_group_ g;
        g.firstChar_ = str[0];
        g.addString(str, addr);
        pBucket_->count_++;
        pBucket_->strGroup_.insert(t, g);
      }
    }
    else
    {
      _string_group_ g;
      g.firstChar_ = str[0];
      g.addString(str, addr);
      pBucket_->count_++;
      pBucket_->strGroup_.push_back(g);
    }

    pBucket_->dirty_ = true;

    
    return pBucket_->size_;
  }

  bool isFull()
  {
    if (pBucket_->size_ >= BUCKET_SIZE)
      return true;
    
    return false;
    
  }

  bool canAddString(const string& str)
  {
    return length()+ONE_STRING_SIZE(str)<=BUCKET_SIZE;
  }
  
  uint32_t length()const
  {
    return pBucket_->size_;
  }

  uint8_t split(Bucket* newBucket)
  {
    if (isPure())
    {
      struct _bucket_* b = new struct _bucket_('a','b');
      struct _bucket_* t = pBucket_;
      pBucket_ = b;
      
      for (str_group_it i=t->strGroup_.begin(); i!=t->strGroup_.end();i++)
      {
        for (str_ptr_it j=(*i).strPtrs_.begin(); j!=(*i).strPtrs_.end();j++)
        {
          addString(*((*j).p_),(*j).contentAddr_ );
          delete (*j).p_;
        }
      }

      delete t;
      pBucket_ = b;
    }
    
    uint32_t left = (uint32_t)(pBucket_->count_/100.00*SPLIT_RATIO);
    uint32_t c = 0;
    
    str_group_it i=pBucket_->strGroup_.begin();
    for (; i!=pBucket_->strGroup_.end();i++)
    {
      c += (*i).strPtrs_.size();
      if (c>=left)
      {
        //found the spliting point
        pBucket_->count_ -= (*i).strPtrs_.size();
        pBucket_->size_ -= newBucket->addStrGroup(*i);
        i = pBucket_->strGroup_.erase(i);
        
        continue;
      }
      
    }
    
    newBucket->setUpBound(getIndexOf(pBucket_->strGroup_.back().firstChar_)+1);
    newBucket->setLowBound(pBucket_->to_);
    pBucket_->to_ = (pBucket_->strGroup_.back()).firstChar_;

    pBucket_->dirty_ = true;
    return pBucket_->to_;
  }

  
  static uint8_t getIndexOf(uint8_t ch)
  {
    if(ALPHABET == a2z)
    {
      if (ch <= 'Z' && ch >='A')
        return ch-'A';
      return ch-ALPHABET[0];
    }

    uint8_t start = 0;
    uint8_t end  = ALPHABET_SIZE -1;
    uint8_t mid = (start + end)/2;
    
    while ( mid<=end && mid>=start)
    {
      if (ALPHABET[mid]==ch)
        return mid;

      if (mid>=end || mid<=start)
        return -1;
      
      if (ALPHABET[mid]<ch)
      {
        start = mid;
        mid = (start + end)/2;
        continue;
      }

      if (ALPHABET[mid]>ch)
      {
        end = mid;
        mid = (start + end)/2;
        continue;
      }
    }

    return -1;
  }

  bool isPure() const
  {
    return (pBucket_->to_ -pBucket_->from_)==0;
  }

  uint8_t getUpBound()const
  {
    return pBucket_->from_;
  }

  uint8_t getLowBound() const
  {
    return pBucket_->to_;
  }

  uint32_t addStrGroup(const _string_group_& g)
  {
    uint32_t size = 0;
    for (const_str_ptr_it j=g.strPtrs_.begin(); j!=g.strPtrs_.end();j++)
      size += ONE_STRING_SIZE(*((*j).p_));

    pBucket_->strGroup_.push_back(g);
    pBucket_->size_ += size;
    pBucket_->dirty_ = true;
    return size;
  }

  void setUpBound(uint8_t ch)
  {
    if (pBucket_->from_!=ch)
      pBucket_->dirty_ = true;
    
    pBucket_->from_ = ch;
  }

  void setLowBound(uint8_t ch)
  {
    if (pBucket_->to_!=ch)
      pBucket_->dirty_ = true;
    
    pBucket_->to_ = ch;
    
  }
  
  
protected:
  FILE* f_;
  struct _bucket_* pBucket_;

  
}
  ;






#endif

/**
 * @file  LinearHashTable.hpp
 * @brief The header file of definition of linear hashing table based on memory.
 */

#ifndef LINEARHASHTABLE_HPP
#define LINEARHASHTABLE_HPP

#include <util/log.h>
#include <types.h>
#include <util/hashFunction.h>
#include <am/am.h>
#include <am/concept/DataType.h>
#include <iostream>
#include <am/concept/DataType.h>
#include <boost/memory.hpp>
#include <boost/static_assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits.hpp>
//#include <boost/utility/enable_if.hpp>
//#include <boost/mpl/not.hpp>
//#include <boost/concept/assert.hpp>

NS_IZENELIB_AM_BEGIN
  

#define screen std::cout 
  
#define END std::endl

typedef std::size_t Size;
typedef std::string String;

class MemStorage
{
}
  ;


  template<class KeyType=Size>
  struct ModeHashFunction
  {
    static inline Size getHashValue(const KeyType& key)
    {
      return -1;
      
    }
    
  };

  template<>
  struct ModeHashFunction<Size>
  {
    static inline Size getHashValue(const Size& key)
    {
      return key;
    }
    
  };

  template<>
  struct ModeHashFunction<String>
  {
    static inline Size getHashValue(const String& key)
    {
      return izenelib::util::HashFunction<String>::convert_key(key)
        % izenelib::util::HashFunction<String>::PRIME;
    }
    
  };

  
    
template<
  class ValueType,
  class KeyType = Size,
  class StorageType = MemStorage,
  int SEGMENT_SIZE = 4,
  int DIRECTORY_SIZE = 1024,
  int expandFctrPerc = 100,
  int shrinkFctrPerc = 50,
  class LockType= ReadWriteLock,
  template<class> class HashFuncType = ModeHashFunction
  >
class LinearHashTable
  : public AccessMethod<KeyType, ValueType, LockType>
{
  
public:
  virtual bool insert(const DataType<KeyType,ValueType>& data) 
  {
    return false;
    
  }

  virtual bool update(const DataType<KeyType,ValueType>& data)
  {
    return false;
    
  }

  virtual ValueType* find(const KeyType& key)
  {
    return NULL;
    
  }

  virtual bool del(const KeyType& key)
  {
    return false;
    
  }

    
}
  ;
  
/**
 *
 * @brief LinearHashTable is based on Per-Ake Larson's work, Dynamic Hash Tables.
 *
 *  It is the fastest hash table that I know and have implemented.
 *	               There are several parameters to tune for faster performance.
 *				   The min/max load factor is vital in the tuning.
 *				   The segment and directory size could be important factors but
 *				   have not been experimented.
 *
 *  One idea is to expedite the growth/shrink process by expanding
 *	                or contracting more than one buckets. It is not clear yet
 *					whether such a move would be beneficial.
 *
 *
 */

template<
  class ValueType,
  class KeyType,
  int SEGMENT_SIZE ,
  int DIRECTORY_SIZE,
  int expandFctrPerc,
  int shrinkFctrPerc,
  class LockType,
  template<class> class HashFuncType 
  >
class LinearHashTable<ValueType,KeyType, MemStorage,
                       SEGMENT_SIZE, DIRECTORY_SIZE,
                       expandFctrPerc, shrinkFctrPerc,
                       LockType,HashFuncType>
  : public AccessMethod<KeyType, ValueType, LockType>
{

  BOOST_STATIC_ASSERT(expandFctrPerc>=1);
  BOOST_STATIC_ASSERT(expandFctrPerc<=100);
  BOOST_STATIC_ASSERT(shrinkFctrPerc>=1);
  BOOST_STATIC_ASSERT(shrinkFctrPerc<=100);
  /*
   * @brief LHTElem is a wrapper class for ValueType and provide next elem
   * for List like behaviour within the LinearHashTable.
   */
  class LHTElem {

  friend class Segment;

  
  public:
    ValueType data_;
    KeyType key_;
    LHTElem* pNext_;
      
    
    LHTElem(const KeyType& key, const ValueType& data) :
      data_(data), pNext_(0), key_(key) {
    }
      
    LHTElem(const LHTElem& source) :
      data_(source.data_), pNext_(source.pNext_), key_(source.key_) {
    }
    
	
    LHTElem& operator=(const LHTElem& source) {
      if (this != &source) {
        data_= source.data_;
        pNext_ = source.next_;
        key_ = source.key_;
            
      }
      return *this;
    }
    
    ~LHTElem() {
    } // next items are deleted in the Segment class.

  friend std::ostream& operator << (std::ostream& out, const LHTElem& ele)
    {
      out<<ele.key_<<":"<<ele.data_;
      return out;
    }

  };
      
  /*
   * @brief Segment is used to support dynamic growth/shrink property of
   * LinearHashTable class.
   */

  class Segment 
  {
  public:
      
    LHTElem *pSeg_[SEGMENT_SIZE];
    
    Segment() {
      for (int i = 0; i < SEGMENT_SIZE; i++)
        pSeg_[i] = NULL;
    }

    ~Segment() {
      for (int i = 0; i < SEGMENT_SIZE; i++) {
        if (pSeg_[i]) {
          LHTElem *curr = pSeg_[i];
          do {
            // delete all the LHTElem objects following its next link.
            LHTElem *next = curr->pNext_;
            //delete curr;
            curr = next;
          } while (curr);
        }
      }
    }
  };

protected:
    
  Size next_; // next bucket to be split
  Size maxp_; // upper bound on next during this expansion
  Size keycount_; // number of records in the table
  Size currentsize_; // current number of buckets
  double minloadfctr_, maxloadfctr_; // lower and upper bound on the load factor
  Segment* pDirectory_[DIRECTORY_SIZE];
  LockType locker_; // for thread-safe
  boost::auto_alloc alloc_;
  
  /**
   *  @brief Returns a hash value of a key.
   */
  Size hash(const KeyType& key) const
  {
    Size h, address;

    h = ModeHashFunction<KeyType>::getHashValue(key);
    address = h % maxp_;
    if (address < next_)
      address = h % (2*maxp_);

    return address;
  }
  
  int compare(const KeyType& a, const KeyType& b) const
  {
    return _compare(a,b, static_cast<boost::is_arithmetic<KeyType>*>(0));
  }
  int _compare(const KeyType& a, const KeyType& b, const boost::mpl::true_*) const
  {
    return a-b;
  }

  int _compare(const KeyType& a, const KeyType& b, const boost::mpl::false_*) const
  {
    BOOST_CONCEPT_ASSERT((KeyTypeConcept<KeyType>));
    return a.compare(b);
  }
  
  /**
   * @brief Expands the table.
   */
  void expand_table()
  {
    Size newaddress, oldsegmentindex, newsegmentindex;
    Segment* oldsegment, *newsegment;
    LHTElem* current, *previous; // for scanning down the old chain
    LHTElem* lastofnew; // points to the last ValueType of the new chain

    // reached maximum size of address space? if so, just continue the chaining.
    if (maxp_ + next_ < DIRECTORY_SIZE * SEGMENT_SIZE) {
      // locate the bucket to be split
      oldsegment = pDirectory_[next_/SEGMENT_SIZE];
      oldsegmentindex = next_ % SEGMENT_SIZE;

      // Expand address space, if necessary create a new Segment<ValueType>
      newaddress = maxp_ + next_;
      newsegmentindex = newaddress % SEGMENT_SIZE;
      if (newsegmentindex == 0)
        pDirectory_[newaddress / SEGMENT_SIZE] = BOOST_NEW(alloc_, Segment);//new Segment();
      newsegment = pDirectory_[newaddress / SEGMENT_SIZE];

      // adjust the state variables
      next_++;
        
      if (next_ == maxp_) {
        maxp_ = 2 * maxp_;
        next_ = 0;
      }

      currentsize_++;

      // relocate records to the new bucket
      current = oldsegment->pSeg_[oldsegmentindex];
      previous = NULL;
      lastofnew = NULL;
      newsegment->pSeg_[newsegmentindex] = NULL;

      while (current != NULL) {
        if (hash(current->key_) == newaddress) {
          // attach it to the end of the new chain
          if (lastofnew == NULL)
            newsegment->pSeg_[newsegmentindex] = current;
          else
            lastofnew->pNext_ = current;
          if (previous == NULL)
            oldsegment->pSeg_[oldsegmentindex] = current->pNext_;
          else
            previous->pNext_ = current->pNext_;
          lastofnew = current;
          current = current->pNext_;
          lastofnew->pNext_ = NULL;
        } else {
          // leave it on the old chain
          previous = current;
          current = current->pNext_;
        }
      }
    }
  }
  
  /**
   * @brief Contracts the table, exactly the opposite of expand_table().
   */
  void contract_table()
  {
    int oldsegmentindex, newsegmentindex;
    Segment *oldsegment, *newsegment;
    LHTElem *current, *previous; // for scanning down the new/current chain

    // Is the table contractable or has more than one segment
    if (currentsize_ > SEGMENT_SIZE) { // there is a bucket to shrink.
      // locate the bucket to free
      currentsize_--;

      oldsegment = pDirectory_[currentsize_/SEGMENT_SIZE];
      oldsegmentindex = currentsize_ % SEGMENT_SIZE;

      // adjust the state variables
      next_--;
        
      if (next_ < 0) {
        maxp_ = maxp_/2;
        next_ = maxp_ - 1;
      }

      newsegment = pDirectory_[next_/SEGMENT_SIZE];
      newsegmentindex = next_ % SEGMENT_SIZE;

      // relocate records to the new bucket
      current = newsegment->pSeg_[newsegmentindex];

      // sacn down the end of the current where the additonal records
      // will be attached to.
      previous = current;
      while (current != NULL) {
        previous = current;
        current = current->pNext_;
      }
      // attach the chain of records to the end of the new bucket
      if (previous) {
        previous->pNext_ = oldsegment->pSeg_[oldsegmentindex];
      } else
        newsegment->pSeg_[newsegmentindex] = oldsegment->pSeg_[oldsegmentindex];

      oldsegment->pSeg_[oldsegmentindex] = NULL;

      // if necessary delete the old Segment<ValueType>
      if (oldsegmentindex == 0) {
        //        delete pDirectory_[currentsize_/SEGMENT_SIZE];
        pDirectory_[currentsize_/SEGMENT_SIZE] = NULL;
      }
    }
  }

  
  void init()
  {
    //  USING_IZENE_LOG();
    
    next_= 0;
    maxp_ = SEGMENT_SIZE;
    keycount_ = 0;
    currentsize_ = SEGMENT_SIZE;
    
    pDirectory_[0] = BOOST_NEW(alloc_, Segment);//new Segment();

    for (int i = 1; i < DIRECTORY_SIZE; i++)
      pDirectory_[i] = NULL;
  }


  virtual bool insert(const DataType<KeyType,ValueType>& data) 
  {
    return insert(LHTElem(data.key, data.value));
  }
  
  bool insert(const LHTElem& elem)
  {
    // first requests the write lock
   	locker_.lock();
    if (maxloadfctr_ * currentsize_ <= keycount_)
      expand_table(); // due for expanding table

    const KeyType& key = elem.key_;
    Size address = hash(key);
    Segment* currentSegment = pDirectory_[address/SEGMENT_SIZE];
    int segIndex = address % SEGMENT_SIZE;
    LHTElem* e = currentSegment->pSeg_[segIndex]; // first on chain
    if (e == NULL)
      currentSegment->pSeg_[segIndex] = BOOST_NEW(alloc_, LHTElem)(elem);//new LHTElem(elem);
    else {
      while (e->pNext_ != NULL) {
        if (compare(e->key_, key)==0) {// duplicate data
          // release the write lock
          locker_.unlock();
          return false;
        }
        e = e->pNext_; // go to the end of the chain
      }
      if (compare(e->key_, key)==0) {
        // release the write lock
        locker_.unlock();
        return false;
      }
      e->pNext_ = BOOST_NEW(alloc_, LHTElem)(elem);//new LHTElem(elem);
    }

    keycount_++; // increment key count by one.
    // release the write lock
    locker_.unlock();
    return true;
  }
  

public:
  typedef LinearHashTable<ValueType,KeyType, MemStorage,SEGMENT_SIZE, DIRECTORY_SIZE,
   expandFctrPerc, shrinkFctrPerc, LockType,HashFuncType>
  Self;
  
  /**
   *  @brief Default constructor, Initialize member variables.
   */
  LinearHashTable():minloadfctr_(shrinkFctrPerc/100.0),maxloadfctr_(expandFctrPerc/100.0)
  {
    init();
  }

  

  /**
   *  @brief The destructor.
   */
  ~LinearHashTable()
  {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
      //delete pDirectory_[i];
      pDirectory_[i] = NULL;
    }
  }
  

  /**
   * @brief Releases the LinearHashTable_ object.
   */
  void release()
  {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
      //      delete pDirectory_[i];
      pDirectory_[i] = NULL;
    }
    init();
  }
  

  int num_items() const {
    return ((LinearHashTable*)this)->get_current_items();
  }
  
  int get_current_items() {
    locker_.lock_shared();
    int currentItems = keycount_;
    locker_.unlock();
    return currentItems;
  }
  
  int num_buckets() const {
    return currentsize_;
  }

  virtual ValueType* find(const KeyType& key)
  {
    locker_.lock_shared();
    int address = hash(key);
    LHTElem* elem =
      pDirectory_[address/SEGMENT_SIZE]->pSeg_[address % SEGMENT_SIZE]; // first on chain
    while (elem != NULL) {
      if (compare(key, elem->key_)==0) {// found!
        // release the read lock
        locker_.unlock_shared();
   
        //      IF_DLOG(compare(key, elem->key_)==0)<<"compare(key, elem->key_)==0";
           
        return &elem->data_;
      } else
        elem = elem->pNext_;
    }
    // release the read lock
    locker_.unlock_shared();
    // not found
    return NULL;
  }
  
  inline const ValueType* find(const KeyType& key) const {
    return (const ValueType*)((LinearHashTable *)this)->find(key);
  }


  virtual bool insert(const KeyType& key, const ValueType& data)
  {
    return insert(LHTElem(key, data));
    
  }

  virtual bool update(const DataType<KeyType,ValueType>& data)
  {
    return update(data.key, data.value);
    
  }

  virtual bool update(const KeyType& key, const ValueType& data)
  {
    locker_.lock();
    bool r = false;
    
    int address = hash(key);
    LHTElem* elem =
      pDirectory_[address/SEGMENT_SIZE]->pSeg_[address % SEGMENT_SIZE]; // first on chain
    while (elem != NULL) {
      if (compare(key, elem->key_)==0) {// found!
        // release the read lock
        elem->data_ = data;
        r = true;
        
      } else
        elem = elem->pNext_;
    }
    // release the read lock
    locker_.unlock();
    // not found

       
    return r;
    
  }
  
  /**
   * @brief Deletes an ValueType.
   * @code
   *	Comments: This method takes the ownership and deletes the data itself
   *            when successful.
   * @endcode
   */
  virtual bool del(const KeyType& key)
  {
    // first requests the write lock
    locker_.lock();
    if (minloadfctr_ * currentsize_ > keycount_)
      contract_table();

    int address = hash(key);
    LHTElem* elem =
      pDirectory_[address/SEGMENT_SIZE]->pSeg_[address % SEGMENT_SIZE]; // first on chain
    LHTElem* prev = elem;

    while (elem != NULL) {
      if (compare(key, elem->key_)) { // found!
        if (prev == elem) //
          pDirectory_[address/SEGMENT_SIZE]->pSeg_[address % SEGMENT_SIZE]
            = prev->pNext_;
        else
          prev->pNext_ = elem->pNext_;

        elem = NULL;//delete elem;
        keycount_--;
        // release the write lock
        locker_.unlock();
        return true;
      } else {
        prev = elem;
        elem = elem->pNext_;
      }
    }
    // release the write lock
    locker_.unlock();
    // not found
    return false;
  }
  

  /**
   * @brief Display information about linear hashing for debug.
   *
   */
  void display(std::ostream& stream) const
  {
    stream << "Member variables: " << std::endl;
    stream << "the number of buckets: " << currentsize_ << std::endl;
    stream << "the number of records: " << keycount_ << std::endl;
    stream << "max load factor: " << maxloadfctr_ << std::endl;
    stream << "min load factor: " << minloadfctr_ << std::endl;
    stream << "max p value: " << maxp_ << std::endl;
    stream << "current p value: " << next_ << std::endl;

    for (int i=0; i<DIRECTORY_SIZE; i++) {
        
      for (int j =0; j<SEGMENT_SIZE; j++) {
        if (i*SEGMENT_SIZE+j>=maxp_+SEGMENT_SIZE)
          return;
          
        stream<<END<<i*SEGMENT_SIZE+j<<":   ";
        if (!pDirectory_[i])
          continue;
        if (!pDirectory_[i]->pSeg_[j])
          continue;
        LHTElem* elem = pDirectory_[i]->pSeg_[j]; // first on chain
        while (elem != NULL) {
          stream<<"|"<<elem->key_<<":"<<elem->data_;
          elem = elem->pNext_;
        }
        stream<<END;
          
      }
    }

  }
  

  template<class Archive> void save(Archive & ar,
                                    const unsigned int version = 0) const {
    ar & keycount_;
      
    for (int i=0; i<DIRECTORY_SIZE; i++) {
      for (int j =0; j<SEGMENT_SIZE; j++) {
        if (!pDirectory_[i])
          continue;
        if (!pDirectory_[i]->pSeg_[j])
          continue;
        LHTElem* elem = pDirectory_[i]->pSeg_[j]; // first on chain
        while (elem != NULL) {
      
          ar & elem->key_;
          ar & elem->data_;
          elem = elem->pNext_;
        }
      }
    }

  }

  template<class Archive> void load(Archive & ar,
                                    const unsigned int version = 0) {
    int n;
    ar & n;

    for (int i =0; i<n; i++) {
      ValueType dat;
      KeyType key;
      ar & key;
      ar & dat;
      insert(key, dat);
    }
  }
  
  void copyTo(AccessMethod<KeyType, ValueType, LockType>& am) const {

    int d = 0;
    for (int i=0; i<DIRECTORY_SIZE; i++) {
      for (int j =0; j<SEGMENT_SIZE; j++) {
        if (!pDirectory_[i])
          continue;
        if (!pDirectory_[i]->pSeg_[j])
          continue;
        LHTElem* elem = pDirectory_[i]->pSeg_[j]; // first on chain
        while (elem != NULL) {
          ++d;
          am.insert(elem->key_, elem->data_);
          elem = elem->pNext;
        }
      }
    }
  }
  
  std::ostream& operator<<(std::ostream& strm) {
    display(strm);
    return strm;
  }




};



NS_IZENELIB_AM_END
#endif


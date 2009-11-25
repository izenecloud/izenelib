/**
 * @file skip-list.hpp
 * @brief The pHeader_ file of SkipList class.
 * @author Kevin Hu
 * @date 2009.11.25
 */
#ifndef _SkipList_H_
#define _SkipList_H_

#include <types.h>
#include <am/am.h>
#include <iostream>
#include <am/concept/DataType.h>
#include <vector>
#include <boost/memory.hpp>
#include <boost/static_assert.hpp>
#include <math.h>

#define screen std::cout 
  
#define END std::endl

typedef std::size_t Size;

class MemStorage
{
}
  ;


NS_IZENELIB_AM_BEGIN

template<class type>
type max_value_of()
{
  return 0;
}

template<>
double max_value_of<double>()
{
  return 1.7* pow(10.0,308);
}

template<>
float max_value_of<float>()
{
  return 3.4* pow(10.,38);
}

template<>
int max_value_of<int>()
{
  return (int)pow(2., sizeof(int)*8.)-1;
}

template<>
long max_value_of<long>()
{
  return (long)pow(2., sizeof(long)*8.)-1;
}


template<>
unsigned long max_value_of<unsigned long>()
{
  return (unsigned long)-1;
}


template<>
unsigned int max_value_of<unsigned int>()
{
  return (unsigned int)-1;
}

template<>
unsigned short max_value_of<unsigned short>()
{
  return (unsigned short)-1;
}
  

template <
  class ValueType,
  class KeyType = unsigned long,
  int MAX_GAP = 3, // 3 means that it's a 1-2-3 skip list. So, 4 means it's 1-2-3-4 skip list
  class StorageType=MemStorage,
  class LockType=NullLock>
class SkipList : public AccessMethod<KeyType, ValueType, LockType>
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
    return false;
  }

  virtual bool del(const KeyType& key)
  {
    return false;
  }
  
}
  ;


/**
 * @brief The definition and implementation of the
 *        SkipList(Actually, it is a deterministic skip list)
 */
template <
  class ValueType,
  class KeyType,
  int MAX_GAP, // 3 means that it's a 1-2-3 skip list. So, 4 means it's 1-2-3-4 skip list
  class LockType>
class SkipList<ValueType, KeyType,MAX_GAP, MemStorage, LockType  >
  : public AccessMethod<KeyType, ValueType, LockType>
{
  BOOST_STATIC_ASSERT(MAX_GAP>=3);
  /**
   * @brief SkipNode reprents a node in a SkipList.
   */
  class SkipNode
  {
  public:
    KeyType key_;
    ValueType* pValue_;
    SkipNode  *pRight_;
    SkipNode  *pDown_;

    SkipNode(const KeyType& key=KeyType(), ValueType* value = 0,
             SkipNode *rt = 0, SkipNode *dt = 0)
      : key_(key), pValue_(value), pRight_(rt), pDown_(dt) {}

    SkipNode(const SkipNode& node)
      : key_(node.key_), pValue_(node.pValue_), pRight_(node.pRight_), pDown_(node.pDown_) {}

    
    friend std::ostream& operator << (std::ostream& out, const SkipNode& node)
      {
        out<<node.key_<<":"<<(node.pValue_==0? "NULL" : *node.pValue_);
        return out;
      }

    ~SkipNode()
    {
    }

    SkipNode* getRightPtr(int times)
    {
      SkipNode  * r = pRight_;
      while(times>1)
      {
        r = r->pRight_;
        times--;

      }
      return r;
      
    }
    

  };

  typedef SkipList<ValueType, KeyType, MAX_GAP,MemStorage, LockType  > Self;
  
public:
  
  /**
   * @brief Construct the tree.
   *
   * inf is the largest DataType
   * and is used to signal failed finds.
   */
  explicit SkipList():m_infinity(max_value_of<KeyType>()),pPool_(alloc_)
  {
    //pBottom_ = new SkipNode();
    pBottom_ = BOOST_NEW(alloc_, SkipNode);
    pBottom_->pRight_ = pBottom_->pDown_ = pBottom_;
    //pTail_   = new SkipNode();
    pTail_   = BOOST_NEW(alloc_, SkipNode);
    pTail_->pRight_ = pTail_;
    pTail_->pDown_ = 0;
    pTail_->key_ = m_infinity;//1000001
    //pHeader_ = new SkipNode(1000000,0, pTail_, pBottom_);
    pHeader_ = BOOST_NEW(alloc_, SkipNode)(m_infinity,0, pTail_, pBottom_);
    size_ = 0;
    //assert(MAX_GAP>3);
  }


  
  /**
   * @brief Copy constructor.
   *
   * Not implemented yet.
   */
  SkipList(const Self & rhs)
  {
    screen << "Copy constructor is unimplemented" << END;
  }

  
  /**
   * @brief Destructor.
   */
  ~SkipList()
  {
    release();
    pHeader_ = 0;
    pTail_ = 0;
    pBottom_ = 0;
  }
  

  /**
   * @brief Find the smallest item in the tree.
   *
   * Return smallest item or m_infinity if empty.
   */
  const ValueType* find_min( ) const
  {
    if( is_empty( ) )
      return m_infinity;

    SkipNode*current = pHeader_;
    while( current->pDown_ != pBottom_ )
      current = current->pDown_;

    return (const ValueType*)current->pValue;
  }

  /**
   * @brief Find the largest item in the tree.
   *
   * Return the largest item or m_infinity if empty.
   */
  const ValueType* find_max( ) const
  {
    if( is_empty( ) )
      return m_infinity;

    SkipNode *current = pHeader_;
    for( ; ; ) {
      if( current->pRight_->pRight_ != pTail_ )
        current = current->pRight_;
      else if( current->pDown_ != pBottom_ )
        current = current->pDown_;
      else
        return (const ValueType*)current->pValue;
    }

    return 0;
  }
  
  
 
  /**
   * @brief Find item x in the tree.
   *
   * Return the matching item or m_infinity if not found.
   */
 
  virtual ValueType* find( const KeyType& v ) 
  {
    
    SkipNode* x;

    x = pHeader_;
    while (x != pBottom_) {
      while (compare(v,  x->key_)>0)
        x = x->pRight_;
      if (x->pDown_ == pBottom_ || v == x->key_)
        return ( (v == x->key_) ? (x->pValue_) : 0);
      x = x->pDown_;
    }

    return 0;
  }
  
  inline const ValueType* find(const KeyType& x)const
  {
    return (const ValueType*)((SkipList*)this)->find(x);
  }

  /**
   * @brief Test if the tree is logically empty.
   *
   * Return true if empty, false otherwise.
   */
  bool is_empty() const
  {
    return (size_ == 0);
  }
  
  /**
   * @brief Print the SkipList.
   */
  void display(std::ostream& stream) const
  {
    SkipNode *level = pHeader_->pDown_;
    Size c = 0;
    
    while(level != pBottom_)
    {
      stream<<END<<"------------------Level:"<<c<<"-----------------"<<END;
      
      SkipNode *current = level;
      while (current!= pTail_)
      {
        stream<<*current<<" ";
        if (current->pDown_ != pBottom_)
          stream<<"-->"<<*current->pDown_;
        stream<<"| ";

        current = current->pRight_;
      }
      c++;
      level  = level->pDown_;
      
    }

    stream<<END;
    
  }
  

  int getSize() const { return size_; }


  /**
   * @brief Make the tree logically empty.
   */
  void release()
  {
//     for (typename std::vector<ValueType*>::iterator i = pPool_.begin(); i!= pPool_.end(); i++)
//      delete *i;
    
    pHeader_->pRight_ = pTail_;
    pHeader_->pDown_ = pBottom_;
    pPool_.clear();
    
  }

  virtual bool insert(const DataType<KeyType,ValueType>& data) 
  {
    return insert(data.key, data.value);
    
  }
  

  /**
   * @brief Insert item x into the SkipList.
   */
  virtual bool insert(const KeyType& key, const ValueType& data)
  {
    
    SkipNode*t, *x;

    x = pHeader_;
    pBottom_->key_ = key;
    //pBottom_->pValue_ = new ValueType(data);
    pBottom_->pValue_ = BOOST_NEW(alloc_, ValueType)(data);
    
    while (x != pBottom_) {
      while (compare(key, x->key_)>0)
        x = x->pRight_;
      if ((x->pDown_ == pBottom_) && (compare(key,x->key_)==0))
      {
        pBottom_->pValue_ = 0;
        return false; // already inserted
          
      }


      if ((x->pDown_ == pBottom_) ||
          (x->key_ == x->pDown_->getRightPtr(MAX_GAP)->key_)) {
        //t = new SkipNode(x->key_, x->pValue_, x->pRight_, x->pDown_->getRightPtr(MAX_GAP-1));
        t = BOOST_NEW(alloc_, SkipNode)(x->key_, x->pValue_, x->pRight_, x->pDown_->getRightPtr(MAX_GAP-1));
        x->pRight_ = t;
        x->key_ = x->pDown_->getRightPtr(MAX_GAP-2)->key_;
        x->pValue_ = x->pDown_->getRightPtr(MAX_GAP-2)->pValue_;
      }
      x = x->pDown_;
    }

    if (pHeader_->pRight_ != pTail_) {
      // t = new SkipNode(m_infinity,0, pTail_, pHeader_);
      t = BOOST_NEW(alloc_, SkipNode)(m_infinity,0, pTail_, pHeader_);
      pHeader_ = t;
    }

    size_++;
    pPool_.push_back(pBottom_->pValue_);
    
    return true;
  }
  
  virtual bool del(const KeyType& v)
  {
    
    SkipNode* x;
    bool r = false;
    
    x = pHeader_;
    while (x != pBottom_) {
      while (compare(v,x->key_)>0)
        x = x->pRight_;
      if (compare(v,x->key_)==0 && x->pValue_!=0)
      {
        x->pValue_ = 0;
        r= true;
        
      }
      
      x = x->pDown_;
    }

    return r;
  }
  
  virtual bool update(const DataType<KeyType,ValueType>& data)
  {
    return update(data.key, data.value);
    
  }
  
  virtual bool update (const KeyType& key, const ValueType& data)
  {
    ValueType* v = find(key);
    if (v == 0)
      return false;
    
    *v= data;
    
    return true;
    
  }
  



private:
  const KeyType m_infinity;//!< the infinite value.

  SkipNode *pHeader_;  //!< The list head
  SkipNode *pBottom_;  //!< the bottom chain head
  SkipNode *pTail_;    //!< the tail
  Size size_;
  std::vector<ValueType*, boost::stl_allocator<ValueType*> > pPool_;
  boost::scoped_alloc alloc_;
  //  std::allocator<ValueType*> alloc_;
  
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
  
  
};

NS_IZENELIB_AM_END
#endif

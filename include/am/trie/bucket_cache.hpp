#ifndef BUCKET_CACHE_HPP
#define BUCKET_CACHE_HPP

#include <stdio.h>
#include "bucket.hpp"
#include <time.h>
#inlcude "cache_strategy.h"

NS_IZENELIB_AM_BEGIN
using namespace std;


template<
  uint64_t CACHE_LENGTH = 1000000,//bytes
  uint32_t BUCKET_SIZE = 8192,//byte
  uint8_t SPLIT_RATIO = 75,
  class CacheType = CachePolicyLARU,
  char* ALPHABET = a2z,
  uint8_t ALPHABET_SIZE = a2z_size
  >
class BucketCache
{
public:
  typedef Bucket<BUCKET_SIZE, SPLIT_RATIO,ALPHABET, ALPHABET_SIZE> NodeType;

protected:
  struct _cache_node_
  {
    CacheType cacheInfo_;
    NodeType* pNode_;

    inline _cache_node_(NodeType* p)
    {
      pNode_ = p;
    }
    inline _cache_node_()
    {
      pNode_ = NULL;
    }
    
    
  }
    ;
#define CACHE_SIZE (CACHE_LENGTH/(sizeof(struct _cache_node_)+NodeType::SIZE_)) 
  
  
public:
  typedef NodeCache <CACHE_LENGTH, CacheType, ALPHABET, ALPHABET_SIZE> SelfType;
  
  class nodePtr
  {
  public:
    nodePtr()
    {      
      pN_ = NULL;
      pC_ = NULL;
      idx_ = (uint32_t)-1;

    }
    
    nodePtr(struct _cache_node_& n, uint32_t idx)
    {
      pN_ = n.pNode_;
      pC_ = &n.cacheInfo_;
      idx_ = idx;
    }
    
    NodeType* operator ->()
    {
      pC_->visit();
      if (pN_ ==NULL)
        cout<<"Node is not exist!\n";//Throw exception
      return pN_;
    }

    void eleminate()
    {
      if (pN_ != NULL)
        delete pN_;

      pN_ = NULL;
    }
    
    uint32_t getIndex() const
    {
      return idx_;
    }

    bool isNull()const
    {
      return pN_==NULL;
    }
    
  protected:
    NodeType* pN_;
    CacheType* pC_;
    uint32_t idx_;
  }
    ;
  
  NodeCache(FILE* f, uint64_t rootAddr)
    :f_(f),rootAddr_(rootAddr),count_(0)
  {
    //cout<<NodeType::SIZE_ <<"///////"<<CACHE_SIZE<<endl;
  }

  uint32_t load()
  {
    return count_;
  }
  
  uint32_t reload()
  {
    for(uint32_t i=0; i<CACHE_SIZE; i++)
    {
      if (nodes[i].pNode_ != NULL)
      {
        delete nodes[i].pNode_;
        nodes[i].pNode_ = NULL;
      }
      
    }
    count_ = 0;
    
    return load();
  }
  

friend ostream& operator << ( ostream& os, const SelfType& node)
  {
    for(uint32_t i=0; i<node.count_; i++)
    {
      os<<"\n****************"<<i<<"****************\n";
      if (node.nodes[i].pNode_==NULL)
        continue;
      os<<node.nodes[i].cacheInfo_;
      os<<*node.nodes[i].pNode_;
    }

    return os;
    
  }
  
  uint32_t findSwitchOut() const
  {
    uint32_t minIdx = 0;
    for (uint32_t i=count_-1; i>0; i--)
    {
      if (nodes[i].pNode_==NULL)
        return i;
      
      if (nodes[i].cacheInfo_.compare(nodes[minIdx].cacheInfo_)<0)
      {
        minIdx = i;
      }
      
    }

    return minIdx;
  }

  uint32_t getCount()const
  {
    return count_;
  }
  
  nodePtr getNodeByMemAddr(uint32_t& memAddr, uint64_t diskAddr)
  {
    if (diskAddr%2==0)
      return nodePtr();
    
    if (memAddr>=count_ && memAddr!=(uint32_t)-1)
      return nodePtr();

    if (memAddr!=(uint32_t)-1)
    {
      NodeType* t = nodes[memAddr].pNode_;
      if (t->getDiskAddr()==diskAddr)
        return nodePtr(nodes[memAddr], memAddr);
    }

    if (count_<CACHE_SIZE)
    {
       
      NodeType* t = new NodeType(f_);
      t->load(diskAddr);
      nodes[count_] = _cache_node_(t);
      
      memAddr = count_;
      
      count_++;
      return nodePtr(nodes[memAddr], memAddr);
    }

    memAddr = findSwitchOut();
    //cout<<"\nswitch out-: "<<memAddr<<endl;
    kickOutNodes(memAddr);
    
    
    NodeType* t = new NodeType(f_);
    t->load(diskAddr);
    nodes[memAddr] = _cache_node_(t);
    return nodePtr(nodes[memAddr], memAddr);
  }


  nodePtr newNode(uint64_t diskAddr)
  {
    if (count_<CACHE_SIZE)
    {
      NodeType* t = new NodeType(f_);
      t->load(diskAddr);
      nodes[count_] = _cache_node_(t);
      nodePtr p(nodes[count_]);
      count_++;
      return p;
    }

    
    uint32_t ret = findSwitchOut();
    kickOutNodes(ret);
    
    NodeType* t = new NodeType(f_);
    t->load(diskAddr);
    nodes[ret] = _cache_node_(t);
    nodePtr p(nodes[ret]);
    return p;

  }
  
  nodePtr newNode()
  {
    if (count_<CACHE_SIZE)
    {
      NodeType* t = new NodeType(f_);
      nodes[count_] = _cache_node_(t);
      nodePtr p(nodes[count_], count_);
      count_++;
      return p;
    }

    
    uint32_t ret = findSwitchOut();
    //<<"\nswitch out: "<<ret<<endl;
    kickOutNodes(ret);
    
    NodeType* t = new NodeType(f_);
    nodes[ret] = _cache_node_(t);
    nodePtr p(nodes[ret], ret);
    return p;

  }


  uint64_t kickOutNodes(uint32_t memAddr)
  {
    if (memAddr==(uint32_t)-1 || nodes[memAddr].pNode_ == NULL)
      return (uint64_t)-1;

    uint64_t ret = nodes[memAddr].pNode_->update2disk();
    delete nodes[memAddr].pNode_;
    nodes[memAddr]=  _cache_node_(NULL);

    return ret;
    
  }

protected:
  FILE* f_;
  uint32_t count_;
  struct _cache_node_ nodes[CACHE_SIZE];
  
}
  ;


NS_IZENELIB_AM_END
#endif

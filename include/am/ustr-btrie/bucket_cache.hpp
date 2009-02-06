#ifndef BUCKET_CACHE_HPP
#define BUCKET_CACHE_HPP

#include <stdio.h>
#include "bucket.hpp"
#include <time.h>
#include "cache_strategy.h"

extern int debug_count;

NS_IZENELIB_AM_BEGIN
using namespace std;


template<
  UString::EncodingType ENCODE_TYPE,
  uint64_t CACHE_LENGTH = 100000000,//bytes
  uint32_t BUCKET_SIZE = 8192,//byte
  uint8_t SPLIT_RATIO = 75,
  class CacheType = CachePolicyLARU,
  unsigned short* ALPHABET = a2z,
  uint32_t ALPHABET_SIZE = a2z_size
  >
class BucketCache
{
public:
  typedef Bucket<ENCODE_TYPE, BUCKET_SIZE, SPLIT_RATIO,ALPHABET, ALPHABET_SIZE> NodeType;

protected:
  struct _cache_node_
  {
    CacheType cacheInfo_;
    NodeType* pNode_;
    bool locked_;

    inline _cache_node_(NodeType* p)
    {
      pNode_ = p;
      locked_ = false;
    }
    inline _cache_node_()
    {
      pNode_ = NULL;
      locked_ = false;
    }
    
    
  }
    ;
#define CACHE_SIZE (CACHE_LENGTH/(sizeof(struct _cache_node_)+NodeType::SIZE_)) 
  
  
public:
  typedef BucketCache <ENCODE_TYPE, CACHE_LENGTH,BUCKET_SIZE, SPLIT_RATIO, CacheType, ALPHABET, ALPHABET_SIZE> SelfType;
  
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
    

    NodeType* pN_;
  protected:
    CacheType* pC_;
    uint32_t idx_;
  }
    ;
  
  BucketCache(FILE* f)
    :f_(f),count_(0)
  {
    //cout<<NodeType::SIZE_ <<"////"<<(CACHE_LENGTH)<<"///"<<CACHE_SIZE<<endl;
    nodes  = new struct _cache_node_[CACHE_SIZE];
  }

  ~BucketCache()
  {
    for (uint32_t i=0; i<CACHE_SIZE; i++)
      if (nodes[i].pNode_ != NULL)
      {
        delete nodes[i].pNode_;
        nodes[i].pNode_ = NULL;
      }
    delete nodes;
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
    for (uint32_t i=0; i<count_; i++)
    {
      if (nodes[i].pNode_==NULL)
        return i;
      
      if ((!nodes[i].locked_) && nodes[i].cacheInfo_.compare(nodes[minIdx].cacheInfo_)<0)
      {
        minIdx = i;
      }
      
    }

    if (minIdx == 0)
      LDBG_<<"Bucket cache is full. Can't find one to switch out";
    
    return minIdx;
  }

  uint32_t getCount()const
  {
    return count_;
  }

  uint32_t findInCache(uint64_t diskAddr)
  {
    for (uint32_t i=0; i<CACHE_SIZE; i++)
    {
      if (nodes[i].pNode_ == NULL)
        continue;

      if(nodes[i].pNode_->getDiskAddr()== diskAddr)
        return i;
    }

    return -1;
    
  }
  
  nodePtr getNodeByMemAddr(uint32_t& memAddr, uint64_t diskAddr)
  {    
    if (diskAddr%2==1)
      return nodePtr();
    
    if (memAddr>=count_ && memAddr!=(uint32_t)-1)
      return nodePtr();

    if (memAddr!=(uint32_t)-1)
    {
      NodeType* t = nodes[memAddr].pNode_;
      if (t->getDiskAddr()==diskAddr)
      {
        //t->load(diskAddr);
        return nodePtr(nodes[memAddr], memAddr);
      }
    }

    uint32_t i = findInCache(diskAddr);
    if (i != (uint32_t)-1)
    {
      memAddr = i;
      return nodePtr(nodes[i], memAddr);
    }
    
    if (count_<CACHE_SIZE)
    {
      //cout<<memAddr<<"  "<<diskAddr<<" "<<debug_count<<" load bucket from disk!\n";
      
      NodeType* t = new NodeType(f_);
      t->load(diskAddr);
      nodes[count_] = _cache_node_(t);
      
      memAddr = count_;
      
      count_++;
      return nodePtr(nodes[memAddr], memAddr);
    }

    //cout<< "getNodeByMemAddr()\n";

    memAddr = findSwitchOut();
    //cout<<"\nswitch out-: "<<memAddr<<endl;
    kickOutNodes(memAddr);
    
    
    NodeType* t = new NodeType(f_);
    t->load(diskAddr);
    nodes[memAddr] = _cache_node_(t);
    return nodePtr(nodes[memAddr], memAddr);
  }

  void lockNode(uint32_t memAddr)
  {
    nodes[memAddr].locked_ = true;
  }

  void unlockNode(uint32_t memAddr)
  {
    nodes[memAddr].locked_ = false;
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

    //cout<<"-------\n";
    uint32_t ret = findSwitchOut();
    
    kickOutNodes(ret);

    NodeType* t = new NodeType(f_);
    
    nodes[ret] = _cache_node_(t);
    nodePtr p(nodes[ret], ret);
    return p;

  }


  uint64_t kickOutNodes(uint32_t memAddr)
  {
    cout<<"Kick out bucket: "<<memAddr<<" "<<count_;
    
    if (memAddr==(uint32_t)-1 || nodes[memAddr].pNode_ == NULL)
      return (uint64_t)-1;

    uint64_t ret = nodes[memAddr].pNode_->update2disk();
    //cout<<nodes[memAddr].cacheInfo_;
    //cout<<nodes[memAddr-1].cacheInfo_;
    //cout<<nodes[memAddr+1].cacheInfo_;
    delete nodes[memAddr].pNode_;
    nodes[memAddr]=  _cache_node_(NULL);

    return ret;
  }

  void flush()
  {
    for (uint32_t i=0; i<CACHE_SIZE; i++)
      if (nodes[i].pNode_ != NULL)
        nodes[i].pNode_->update2disk();    
  }

protected:
  FILE* f_;
  uint32_t count_;
  struct _cache_node_* nodes;
  
}
  ;


NS_IZENELIB_AM_END
#endif

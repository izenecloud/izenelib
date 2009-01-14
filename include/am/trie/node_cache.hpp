#ifndef NODE_CACHE_HPP
#define NODE_CACHE_HPP

#include <stdio.h>
#include <alphabet_node.hpp>
#include <time.h>

class CachePolicyLRU//latest rare used
{
  clock_t time_;
public:
  int compare(const CachePolicyLRU& t)
  {
    return time_ - t.time_;
  }

  void visit()
  {
    time_ = clock();
  }
  
  
}
  ;

class CachePolicyLU//least used
{
  uint64_t visit_count_;
  
public:
  CachePolicyLU()
  {
    visit_count_ = 0;
  }
  
  int compare(const CachePolicyLU& t)
  {
    return visit_count_ - t.vist_count_;
  }

  void visit()
  {
    visit_count_++;
  }
  
  
}
  ;

class CachePolicyLARU//least and rarest used
{
  uint64_t visit_count_;
  clock_t time_;
  
public:
  CachePolicyLARU()
  {
    visit_count_ = 0;
  }
  
  int compare(const CachePolicyLU& t)
  {
    return (visit_count_*time_ - t.vist_count_*t.time_)/CLOCKS_PER_SEC;
  }

  void visit()
  {
    time_ = clock();
    visit_count_++;
  }
  
  
}
  ;

template<
  uint64_t CACHE_LENGTH = 1000000,//bytes
  class CacheType = CachePolicyLARU,
  char* ALPHABET = a2z
  >
class NodeCache
{
public:
  typedef AlphabetNode<ALPHABET> NodeType;

protected:
  struct _cache_node_
  {
    CacheType cacheInfo_;
    NodeType* pNode_;

    inline _cache_node_(NodeType* p)
    {
      pNode_ = p;
    }
    
  }
    ;
#define CACHE_SIZE CACHE_LENGTH/(sizeof(struct _cache_node_)+sizeof(AlphabetNode)) 
  
  
public:
  NodeCache(FILE* f, uint64_t rootAddr)
    :f_(f),rootAddr_(rootAddr),count_(0)
  {
  }

  uint64_t load()
  {
    vector<uint64_t>& indexes;
    indexes.push_back(rootAddr_);

    
    NodeType* t = new NodeType(f);
    t->load(rootAddr_);
    nodes[count_] = _cache_node_(t);
    count_++;
      
    while (indexes.size()>0)
    {
      load_(indexes);
    }

    return count_;
  }
  
  void load_(vector<uint64_t>& indexes)
  {
    uint64_t idx = indexes.front();
    indexes.pop_front();
    
    NodeType* n = nodes[idx].pNode_ ;
    
    for (uint8_t i=0; i<n->getSize();i++)
    {
      if (n->getDiskAddr(i)==(uint64_t)-1)
        continue;
      
      if (n->getDiskAddr(i)%2==0)
        continue;
      
      NodeType* t = new NodeType(f);
      t->load(n->getDiskAddr(i));
      nodes[count_] = _cache_node_(t);
      
      indexes.push_back(count_);
      n->setMemAddr(i, count_);
      
      count_++;
      if (count_>=CACHE_SIZE)
      {
        indexes.clear();
        return;
      }
      
    }
  }

  uint64_t findSwitchOut()
  {
    uint64_t minIdx = 0;
    for (uint64_t i=1; i<count_; i++)
    {
      if (nodes[i].cacheInfo_.compare(nodes[minIdx].cacheInfo_)<0)
      {
        minIdx = i;
      }
      
    }

    return minIdx;
  }

  NodeType* getNodeByMemAddr(uint64_t& memAddr, uint64_t diskAddr)
  {
    if (diskAddr%2==0)
      return NULL;
    
    if (memAddr>=count_ && memAddr!=(uint64_t)-1)
      return NULL;

    if (memAddr!=(uint64_t)-1)
    {
      NodeType* t = nodes[memAddr].pNode_;
      if (t->getDiskAddr()==diskAddr)
        return t;
    }

    if (count_<CACHE_SIZE)
    {
       
      NodeType* t = new NodeType(f_);
      t->load(n->getDiskAddr(i));
      nodes[count_] = _cache_node_(t);
      
      memAddr = count_;
      
      count_++;
      return t;
    }

    memAddr = findSwitchOut();
    switchOutNodes(memAddr);
    
    
    NodeType* t = new NodeType(f_);
    nodes[memAddr] = _cache_node_(t);
    return t;
  }

  
  uint32_t addNode(uint32_t diskAddr)
  {
    
    if (count_<CACHE_SIZE)
    {
      NodeType* t = new NodeType(f_);
      t->load(diskAddr);
      nodes[count_] = _cache_node_(t);
      
      count_++;
      return count_;
    }

    
    uint32_t ret = findSwitchOut();
    switchOutNodes(ret);
        
    NodeType* t = new NodeType(f_);
    nodes[ret] = _cache_node_(t);
    return ret;

  }

    
  uint32_t newNode()
  {
    if (count_<CACHE_SIZE)
    {
      NodeType* t = new NodeType(f_);
      nodes[count_] = _cache_node_(t);
      count_++;
      return count_;
    }

    
    uint32_t ret = findSwitchOut();
    switchOutNodes(ret);
    
    NodeType* t = new NodeType(f_);
    nodes[ret] = _cache_node_(t);
    return ret;

  }

protected:
  FILE* f_;
  uint32_t rootAddr_;
  uint32_t count_;
  struct _cache_node_ nodes[CACHE_SIZE];
  
}
  ;



#endif

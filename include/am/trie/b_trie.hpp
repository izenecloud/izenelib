#ifndef B_TRIE_HPP
#define B_TRIE_HPP

#include <alphabet_node.hpp>
#include <bucket.hpp>
#include <node_cache.hpp>
#include <string>
#include <stdio.h>

using namespace std;

NS_IZENELIB_AM_BEGIN
/**
 **/
template<
  class DataType = string,
  //------------bucket property-------------
  uint32_t BUCKET_SIZE = 8192,//byte
  uint8_t SPLIT_RATIO = 75,
  //----------node cache-----------
  uint64_t CACHE_LENGTH = 1000000,//bytes
  class CacheType = CachePolicyLARU,
  char* ALPHABET = a2z
  >
class BTrie
{
  typedef Bucket<DataType, BUCKET_SIZE, SPLIT_RATIO> BucketType;
  typedef NodeCache<CACHE_LENGTH, CacheType, ALPHABET> NodeCacheType;
  typedef typename NodeCacheType::NodeType AlphabetNodeType;

public:
  BTrie(const string& filename)
    :pNodeCache_(NULL), pBucket(NULL)
  {
    f_ = fopen(filename.c_str(), "r+");
    if (f_ == NULL)
    {
      f_ = fopen(filename.c_str(), "w+");
      pNodeCache_ = new NodeCache(f, 1);
      pNodeCache_->newNode();
    }
    else
    {
      pNodeCache_ = new NodeCache(f, 1);
      pNodeCache_->load();
    }
  }

  bool insert(const string& str, const DataType& content)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }

    uint32_t idx = 0;
    uint64_t addr = 1;
    AlphabetNodeType* n_1 = NULL;
    uint32_t idx_1 = 0;
    for (size_t i=0; i<str.length(); i++)
    {
      if (addr == (uint64_t)-1)
      {
        //throw exception
        return false;
      }
      
      if (addr%2==0)
      {
        //load bucket
        if(pBucket_==NULL)
          pBucket_ = new Bucket(f_);

        pBucket->load(addr);
        pBucket_->addString(str.substr(i-1));
        return pBucket_->update2disk();
      }
      
      AlphabetNodeType* n = pNodeCache_->getNodeByMemAddr(idx, addr);
      
      if (n_1!=NULL)
      {
        n_1->setMemAddr(idx_1, idx);
      }
      idx_1 = idx;
      n_1 = n;
      
      uint8_t ch = n->getIndexOf(str[i]);
      idx = n->getMemAddr(ch);
      addr = n->getDiskAddr(ch);
    }

    //in hash table
    return true;
  }

  bool del(const string& str)
  {
  }

  bool update(const string& str, const DataType& content)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }

    uint32_t idx = 0;
    uint64_t addr = 1;
    AlphabetNodeType* n_1 = NULL;
    uint32_t idx_1 = 0;
    for (size_t i=0; i<str.length(); i++)
    {
      if (addr == (uint64_t)-1)
      {
        //throw exception
        return false;
      }
      
      if (addr%2==0)
      {
        //load bucket
        if(pBucket_==NULL)
          pBucket_ = new Bucket(f_);

        pBucket->load(addr);
        pBucket_->updateContent(str.substr(i-1),content);
        return pBucket_->update2disk();
      }
      
      AlphabetNodeType* n = pNodeCache_->getNodeByMemAddr(idx, addr);
      
      if (n_1!=NULL)
      {
        n_1->setMemAddr(idx_1, idx);
      }
      idx_1 = idx;
      n_1 = n;
      
      uint8_t ch = n->getIndexOf(str[i]);
      idx = n->getMemAddr(ch);
      addr = n->getDiskAddr(ch);
    }

    //in hash table
    return true;
  }

  void query(const string& str, DataType& content)
  {
    
    if (pNodeCache_==NULL)
    {
      return false;
    }

    uint32_t idx = 0;
    uint64_t addr = 1;
    AlphabetNodeType* n_1 = NULL;
    uint32_t idx_1 = 0;
    for (size_t i=0; i<str.length(); i++)
    {
      if (addr == (uint64_t)-1)
      {
        //throw exception
        return false;
      }
      
      if (addr%2==0)
      {
        //load bucket
        if(pBucket_==NULL)
          pBucket_ = new Bucket(f_);

        pBucket->load(addr);
        return pBucket_->getContentBy(str.substr(i-1), content);
      }
      
      AlphabetNodeType* n = pNodeCache_->getNodeByMemAddr(idx, addr);
      
      if (n_1!=NULL)
      {
        n_1->setMemAddr(idx_1, idx);
      }
      idx_1 = idx;
      n_1 = n;
      
      uint8_t ch = n->getIndexOf(str[i]);
      idx = n->getMemAddr(ch);
      addr = n->getDiskAddr(ch);
    }

    //in hash table
  }
  
protected:
  FILE* f_;
  NodeCacheType* pNodeCache_;
  BucketType* pBucket_;
}
  ;

NS_IZENELIB_AM_END
#endif

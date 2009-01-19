#ifndef B_TRIE_HPP
#define B_TRIE_HPP

#include "alphabet_node.hpp"
#include "bucket.hpp"
#include "node_cache.hpp"
#include <string>
#include <stdio.h>

using namespace std;

NS_IZENELIB_AM_BEGIN
/**
 **/
template<
  //------------bucket property-------------
  uint32_t BUCKET_SIZE = 8196,//byte
  uint8_t SPLIT_RATIO = 75,
  //----------node cache-----------
  uint64_t CACHE_LENGTH = 1000000,//bytes
  class CacheType = CachePolicyLARU,
  char* ALPHABET = a2z,
  uint8_t ALPHABET_SIZE = a2z_size
  >
class BTrie
{
  typedef BTrie<BUCKET_SIZE, SPLIT_RATIO, CACHE_LENGTH, CacheType, ALPHABET, ALPHABET_SIZE> SelfType;
  typedef Bucket<BUCKET_SIZE, SPLIT_RATIO, ALPHABET, ALPHABET_SIZE> BucketType;
  typedef NodeCache<CACHE_LENGTH, CacheType, ALPHABET, ALPHABET_SIZE> NodeCacheType;
  typedef typename NodeCacheType::nodePtr AlphabetNodePtr;

public:
  BTrie(const string& filename)
    :pNodeCache_(NULL), pBucket_(NULL)
  {
    string bstr = filename+".buk";
    string nstr = filename + ".nod";
    
    nodf_ = fopen(nstr.c_str(), "r+");
    if (nodf_ == NULL)
    {
      nodf_ = fopen(nstr.c_str(), "w+");
      pNodeCache_ = new NodeCacheType(nodf_, 1);
      AlphabetNodePtr n = pNodeCache_->newNode();
      n->add2disk();
    }
    else
    {
      pNodeCache_ = new NodeCacheType(nodf_, 1);
      pNodeCache_->load();
    }

    //////////////
    bukf_ = fopen(bstr.c_str(), "r+");
    if (bukf_ == NULL)
    {
      bukf_ = fopen(bstr.c_str(), "w+");
    }
  }

  bool insert(const string& str, uint64_t contentAddr)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }
    
    uint64_t addr = 1;
    AlphabetNodePtr n_1 ;
    uint32_t rootIdx = 0;
    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(rootIdx, addr);//root node address is 1 in disk and 0 in cache
    for (size_t i=0; i<str.length()-1; i++)
    {
      if (n.isNull())
      {
        //throw exception
        return false;
      }

      uint8_t idx = n->getIndexOf(str[i]);
      uint64_t diskAddr = n->getDiskAddr(idx);
      uint32_t memAddr = n->getMemAddr(idx);

      if(diskAddr == (uint64_t)-1 )
      {
        // new bucket, then add rest string into bucket, return.
        //cout<<"new bucket, then add rest string into bucket, return.\n";
        
        newBucket();
        pBucket_->addString(str.substr(i), contentAddr);
        uint64_t addr =pBucket_->add2disk();
        n->setAllDiskAddr( addr);
        //n = pNodeCache_->getNodeByMemAddr(rootIdx, 1);//
        //n->display(cout);
        //cout<<*pBucket_;
        return true;
      }

      
      if (diskAddr%2==0)
      {
        //load bucket
        
        //cout<<diskAddr<<" load bucket;\n";
        //loadBucket(0);
        //cout<<*pBucket_;
        
        loadBucket(diskAddr);
        //if (diskAddr == 336200)
        // cout<<*pBucket_;
        //cout<<diskAddr<<"load bucket;\n";
        //cout<<diskAddr<<str<<" :bucket add string;\n";
        pBucket_->addString(str.substr(i), contentAddr);
        //cout<<*pBucket_;
        if (!pBucket_->canAddString("abc"))
        {
          //cout<<str<<" ----->fullllll!"<<endl;

          //if (diskAddr == 336200)
          // cout<<"------------------ bucket full, split it*********************\n";
          // if bucket full, split it;
          splitBucket(n, diskAddr, idx);
          //if (diskAddr == 336200)
          //cout<<*pBucket_;
        }

        //n->display(cout);
        
        pBucket_->update2disk();
        
        return true;
      }

      //cout<<"**********\n";
      n_1 = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
      
      if (!n_1.isNull())
      {
        n->setMemAddr(idx, memAddr);
      }

      n = n_1;
    }

    //in hash table
    return true;
  }

  void splitBucket( AlphabetNodePtr& n, uint64_t diskAddr, uint8_t idx)
  {
    uint8_t up = pBucket_->getUpBound();
    uint8_t low = pBucket_->getLowBound();
    //cout<<up<<" "<<low<<"  "<<pBucket_->getStrGroupAmount()<<" 8888888888\n";
    
    if (pBucket_->getStrGroupAmount()==1 && up!=low)
    {
      //if only have one string group
      uint8_t splitPoint = pBucket_->getGroupChar(0);
      //cout<<splitPoint<<"  00000000000\n";
      if (up!=splitPoint)
      {
        uint8_t beforeSplitPoint = ALPHABET[n->getIndexOf(splitPoint)-1];
        BucketType bucket(bukf_);
        bucket.setUpBound(up);
        bucket.setLowBound(beforeSplitPoint);
        n->setDiskAddr(up, beforeSplitPoint, bucket.add2disk());
      }
      
      if (splitPoint != low)
      {
        uint8_t afterSplitPoint = ALPHABET[n->getIndexOf(splitPoint)+1];
        BucketType bucket(bukf_);
        bucket.setUpBound(afterSplitPoint);
        bucket.setLowBound(low);
        n->setDiskAddr(afterSplitPoint, low, bucket.add2disk());
      }
      pBucket_->setUpBound(splitPoint);
      pBucket_->setLowBound(splitPoint);
      up = low = splitPoint;
    }
          
    BucketType bucket(bukf_);
    uint8_t splitPoint = pBucket_->split(&bucket);
    //cout<<"splitting point: "<<splitPoint<<endl;
    uint64_t newBktAddr =  bucket.add2disk();
    //if (diskAddr == 336200)cout<<bucket;
    
    if (up == low)
    {
      //cout<<"pBucket_->isPure()\n";
            
      AlphabetNodePtr newNode = pNodeCache_->newNode();
      newNode->setDiskAddr(ALPHABET[0], splitPoint, diskAddr);
      newNode->setDiskAddr(bucket.getUpBound(), ALPHABET[ALPHABET_SIZE-1], newBktAddr);
      n->setDiskAddr(idx, newNode->add2disk());
      n->setMemAddr(idx, newNode.getIndex());
    }
    else
    {
      n->setDiskAddr(up, splitPoint, diskAddr);
      n->setDiskAddr(bucket.getUpBound(), low, newBktAddr);
    }

  }
  
  bool del(const string& str)
  {
    return false;
  }

  bool update(const string& str, uint64_t contentAddr)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }

    uint32_t idx = 0;
    uint64_t addr = 1;
    AlphabetNodePtr n_1;
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
        loadBucket(addr);
        
        pBucket_->updateContent(str.substr(i-1),contentAddr);
        return pBucket_->update2disk();
      }
      
      AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(idx, addr);
      
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

  uint64_t query(const string& str)
  {
    
    if (pNodeCache_==NULL)
    {
      return false;
    }

    
    uint64_t addr = 1;
    uint32_t rootIdx = 0;
    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(rootIdx, addr);//root node address is 1 in disk and 0 in cache
    for (size_t i=0; i<str.length()-1; i++)
    {
      cout<<str[i]<<"====>\n";
      
      if (n.isNull())
      {
        //throw exception
        cout<<"Eception 1\n";
        return false;
      }

      uint8_t idx = n->getIndexOf(str[i]);
      uint64_t diskAddr = n->getDiskAddr(idx);
      uint32_t memAddr = n->getMemAddr(idx);

      if (diskAddr == (uint64_t)-1)
      {
        //throw exception
        cout<<"Eception 2\n";
        return (uint64_t)-1;
      }

      
      if (diskAddr%2==0)
      {
        //load bucket
        if(pBucket_==NULL)
          pBucket_ = new BucketType(bukf_);

        pBucket_->load(diskAddr);
        cout<<str.substr(i)<<"++++++>";
        return pBucket_->getContentBy(str.substr(i));
      }
      
      n = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);

    }

    //in hash table

    return 0;
  }

  uint32_t getNodeAmount() const
  {
    return pNodeCache_->getCount();
  }

  void display(ostream& os, const string& str)
  {
    uint32_t root = 0;
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(root, 1);
    n->display(os);
    
    for (size_t i=0; i<str.length(); i++)
    {
      os<<endl<<"=========>>>> "<<str[i]<<endl;
      uint64_t diskAddr = n->getDiskAddr(n->getIndexOf(str[i]));
      uint32_t memAddr = n->getMemAddr(n->getIndexOf(str[i]));

      if (diskAddr == (uint64_t)-1)
      {
        os<<"The chain is broken!\n!";
        return;
      }

      if (diskAddr%2==0)
      {
        loadBucket(diskAddr);
        os<<*pBucket_;
        return;
      }
      
      n = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
    }
    
  }

  bool loadBucket(uint64_t addr)
  {
    if(pBucket_== NULL)
          pBucket_ = new BucketType(bukf_);
    return pBucket_->load(addr);
  }

  void newBucket()
  {
    if (pBucket_!= NULL)
      delete pBucket_;

    pBucket_ = new BucketType(bukf_);
  }
  
// friend ostream& operator << ( ostream& os, const SelfType& node)
//   {
//   }
  
protected:
  FILE* nodf_;
  FILE* bukf_;
  NodeCacheType* pNodeCache_;
  BucketType* pBucket_;
}
  ;

NS_IZENELIB_AM_END
#endif

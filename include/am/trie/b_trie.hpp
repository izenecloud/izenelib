#ifndef B_TRIE_HPP
#define B_TRIE_HPP

#include "alphabet_node.hpp"
#include "bucket.hpp"
#include "node_cache.hpp"
#include "bucket_cache.hpp"
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
  //----------bucket cache---------
  uint64_t BUCKET_CACHE_LENGTH = 10000000000,//bytes
  class BucketCachePolicy= CachePolicyLARU,
  //----------node cache-----------
  uint64_t NODE_CACHE_LENGTH = 1000000,//bytes
  class NodeCachePolicy = CachePolicyLARU,
  
  char* ALPHABET = a2z,
  uint8_t ALPHABET_SIZE = a2z_size
  >
class BTrie
{
  typedef BTrie<BUCKET_SIZE, SPLIT_RATIO, BUCKET_CACHE_LENGTH, BucketCachePolicy, NODE_CACHE_LENGTH, NodeCachePolicy, ALPHABET, ALPHABET_SIZE> SelfType;
  typedef Bucket<BUCKET_SIZE, SPLIT_RATIO, ALPHABET, ALPHABET_SIZE> BucketType;
  typedef NodeCache<NODE_CACHE_LENGTH, NodeCachePolicy, ALPHABET, ALPHABET_SIZE> NodeCacheType;
  typedef BucketCache<BUCKET_CACHE_LENGTH, BUCKET_SIZE, SPLIT_RATIO, BucketCachePolicy, ALPHABET, ALPHABET_SIZE> BucketCacheType;
  typedef typename NodeCacheType::nodePtr AlphabetNodePtr;
  typedef typename BucketCacheType::nodePtr BucketPtr;

public:
  BTrie(const string& filename)
    :pNodeCache_(NULL)
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
    pBucketCache_ =  new BucketCacheType(bukf_ );
  }

  ~BTrie()
  {
    if (pNodeCache_!= NULL)
      delete pNodeCache_;
    if ( pBucketCache_ != NULL)
      delete pBucketCache_;
  }
  
  void flush()
  {
    pBucketCache_->flush();
    pNodeCache_ ->flush();
  }

  bool insert(string* pStr, uint64_t contentAddr)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }
    
    uint64_t addr = 1;
    AlphabetNodePtr n_1 ;
    uint32_t memAddr = 0;
    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(memAddr, addr);//root node address is 1 in disk and 0 in cache
        
    for (; pStr->length()>0; )
    {
      if (n.isNull())
      {
        //throw exception
        return false;
      }

      pNodeCache_->lockNode(n.getIndex());
      uint8_t idx = n->getIndexOf((*pStr)[0]);
      uint64_t diskAddr = n->getDiskAddr(idx);
      memAddr = n->getMemAddr(idx);

      if(diskAddr == (uint64_t)-1 )
      {
        // new bucket, then add rest string into bucket, return.
        //cout<<"new bucket, then add rest string into bucket, return.\n";
        
        BucketPtr b = newBucket();
        //pStr->erase(0,1);
        b->addString(pStr, contentAddr);
        uint64_t addr =b->add2disk();
        n->setAllDiskAddr( addr);
        n->setAllMemAddr(b.getIndex());
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
        
        BucketPtr b = loadBucket(memAddr, diskAddr);
        pBucketCache_->lockNode(memAddr);

        //b->display(cout);
        
        n->setMemAddr(idx, memAddr);
        //if (diskAddr == 336200)
        // cout<<*pBucket_;
        //cout<<diskAddr<<" add string!;\n";
        //cout<<diskAddr<<str<<" :bucket add string;\n";
        //pStr->erase(0,1);
        b->addString(pStr, contentAddr);
        //cout<<diskAddr<<" added string!;\n";
        
        //cout<<*pBucket_;
        if (b->isFull())
        {
          //cout<<*pStr<<" ----->fullllll!"<<endl;

          //if (diskAddr == 336200)
          // cout<<"------------------ bucket full, split it*********************\n";
          // if bucket full, split it;
          splitBucket(b, n, diskAddr, idx);
          //cout<<"splitted...\n";
          
          //if (diskAddr == 336200)
          //cout<<*pBucket_;
        }

        //n->display(cout);
        
        //b->update2disk();
        pBucketCache_->unlockNode(memAddr);
        return true;
      }

      //cout<<"**********\n";
      n_1 = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
      
      if (!n_1.isNull())
      {
        n->setMemAddr(idx, memAddr);
      }
      pNodeCache_->unlockNode(n.getIndex());

      n = n_1;
      pStr->erase(0,1);
    }

    //in hash table
    return true;
  }

  void splitBucket(BucketPtr& b,  AlphabetNodePtr& n, uint64_t diskAddr, uint8_t idx)
  {
    uint8_t up = b->getUpBound();
    uint8_t low = b->getLowBound();
    //cout<<up<<" "<<low<<"  "<<b->getStrGroupAmount()<<" 8888888888\n";
    
    if (b->getStrGroupAmount()==1 && up!=low)
    {
      //if only have one string group
      uint8_t splitPoint = b->getGroupChar(0);
      //cout<<splitPoint<<"  00000000000\n";
      if (up!=splitPoint)
      {
        uint8_t beforeSplitPoint = ALPHABET[n->getIndexOf(splitPoint)-1];
        BucketPtr b1 =  pBucketCache_->newNode();
        b1->setUpBound(up);
        b1->setLowBound(beforeSplitPoint);
        n->setDiskAddr(up, beforeSplitPoint, b1->add2disk());
        n->setMemAddr(up, beforeSplitPoint, b1.getIndex());
      }
      
      if (splitPoint != low)
      {
        uint8_t afterSplitPoint = ALPHABET[n->getIndexOf(splitPoint)+1];
        BucketPtr b2  = pBucketCache_->newNode();
        b2->setUpBound(afterSplitPoint);
        b2->setLowBound(low);
        n->setDiskAddr(afterSplitPoint, low, b2->add2disk());
        n->setMemAddr(afterSplitPoint, low, b2.getIndex());
      }
      b->setUpBound(splitPoint);
      b->setLowBound(splitPoint);
      up = low = splitPoint;
    }
    
    BucketPtr bu = pBucketCache_->newNode();
    //cout<<"splitting!\n";
    uint8_t splitPoint = b->split(bu.pN_);
    //cout<<"splitting point: "<<splitPoint<<endl;
    uint64_t newBktAddr =  bu->add2disk();
    
    if (up == low)
    {
      //cout<<pNodeCache_->getCount();
      //cout<<"pBucket_->isPure() \n";
            
      AlphabetNodePtr newNode = pNodeCache_->newNode();
      newNode->setDiskAddr(ALPHABET[0], splitPoint, diskAddr);
      newNode->setDiskAddr(bu->getUpBound(), ALPHABET[ALPHABET_SIZE-1], newBktAddr);
      
      newNode->setMemAddr(ALPHABET[0], splitPoint, b.getIndex());
      newNode->setMemAddr(bu->getUpBound(), ALPHABET[ALPHABET_SIZE-1], bu.getIndex());
      
      n->setDiskAddr(idx, newNode->add2disk());
      n->setMemAddr(idx, newNode.getIndex());
    }
    else
    {
      //cout<<"pBucket_ is not Pure\n";
      n->setDiskAddr(up, splitPoint, diskAddr);
      n->setMemAddr(up, splitPoint, b.getIndex());
      n->setDiskAddr(bu->getUpBound(), low, newBktAddr);
      n->setMemAddr(bu->getUpBound(), low, bu.getIndex());
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
        BucketPtr b = pBucketCache_->getNodeByMemAddr(idx, addr);
        
        b->updateContent(str.substr(i-1),contentAddr);
        return b->update2disk();
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
        BucketPtr b = pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
        //b->display(cout);
        cout<<str.substr(i)<<"++++++>";
        return b->getContentBy(str.substr(i));
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
        BucketPtr b = pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
        b->display(cout);
        return;
      }
      
      n = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
    }
    
  }

  BucketPtr loadBucket(uint32_t& memAddr, uint64_t diskAddr)
  {
    return pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
  }

  BucketPtr newBucket()
  {
    return pBucketCache_->newNode();
    
  }
  
// friend ostream& operator << ( ostream& os, const SelfType& node)
//   {
//   }
  
protected:
  FILE* nodf_;
  FILE* bukf_;
  BucketCacheType* pBucketCache_;
  NodeCacheType* pNodeCache_;

}
  ;

NS_IZENELIB_AM_END
#endif

#ifndef STRING_B_TRIE_HPP
#define STRING_B_TRIE_HPP

#include "alphabet_node.hpp"
#include "bucket.hpp"
#include "node_cache.hpp"
#include "bucket_cache.hpp"
#include <string>
#include <stdio.h>
#include <am/cccr_string_hash_table/cccr_str_hash_table.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 *@class BTrie
 *B-trie is a kind of multi-way tree. There’re two kinds of nodes in that
 *tree. One is normal node which is an alphabet with pointer actually.
 *We just call them ’Node’. The other is called bucket which stores
 *bunches of strings suffix. The prefix is consumed by nodes. And, I
 *store nodes and bucket in 2 different separate files. The insertion of
 *a strings may involve splitting bucket. This string b-trie means the value field could be string.
 **/
template<

  char* ALPHABET = a2z,
  uint8_t ALPHABET_SIZE = a2z_size,

  //------------bucket property-------------
  uint32_t BUCKET_SIZE = 8196,//byte
  uint8_t SPLIT_RATIO = 75,
  
  //--------------hash table-------------
  size_t ENTRY_SIZE= 10000,
  class HASH_FUNCTION = simple_hash,
  int EXPAND = PAGE_EXPANDING,
  //----------bucket cache---------
  uint64_t BUCKET_CACHE_LENGTH = 10000000000,//bytes, it must be larger than 2 bucket size
  class BucketCachePolicy= CachePolicyLARU,
  //----------node cache-----------
  uint64_t NODE_CACHE_LENGTH = 100000000,//bytes, it must be larger than 3 node size.
  class NodeCachePolicy = CachePolicyLARU
  >
class StringBTrie
{
  typedef StringBTrie<ALPHABET, ALPHABET_SIZE, BUCKET_SIZE, SPLIT_RATIO, ENTRY_SIZE, HASH_FUNCTION,PAGE_EXPANDING, BUCKET_CACHE_LENGTH, BucketCachePolicy, NODE_CACHE_LENGTH, NodeCachePolicy> SelfType;
  typedef Bucket<BUCKET_SIZE, SPLIT_RATIO, ALPHABET, ALPHABET_SIZE> BucketType;
  typedef NodeCache<NODE_CACHE_LENGTH, NodeCachePolicy, ALPHABET, ALPHABET_SIZE> NodeCacheType;
  typedef BucketCache<BUCKET_CACHE_LENGTH, BUCKET_SIZE, SPLIT_RATIO, BucketCachePolicy, ALPHABET, ALPHABET_SIZE> BucketCacheType;
  typedef typename NodeCacheType::nodePtr AlphabetNodePtr;
  typedef typename BucketCacheType::nodePtr BucketPtr;
  typedef CCCR_StrHashTable<string, uint64_t, ENTRY_SIZE, HASH_FUNCTION,PAGE_EXPANDING> HashTable;

public:

  /**
   *@param filename Name of file stores trie data. It will generate 3 files. The suffix of them are '.buk', '.nod', '.has'.
   * They stands for bucket file, trie node file and hash table file respectively.
   **/
  StringBTrie(const string& filename)
    :pNodeCache_(NULL)
  {
    string bstr = filename+".buk";
    string nstr = filename + ".nod";
    hash_file_ = filename + ".has";
    string vstr = filename + ".val";
    bool isload = false;
    
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
      isload = true;
    }

    //////////////
    bukf_ = fopen(bstr.c_str(), "r+");
    if (bukf_ == NULL)
    {
      bukf_ = fopen(bstr.c_str(), "w+");
    }
    pBucketCache_ =  new BucketCacheType(bukf_ );

    valuef_ = fopen(vstr.c_str(), "r+");
    if (valuef_ == NULL)
    {
      valuef_ = fopen(vstr.c_str(), "w+");
    }
    
    if (isload)
      load();
  }

  ~StringBTrie()
  {
    if (pNodeCache_!= NULL)
      delete pNodeCache_;
    if ( pBucketCache_ != NULL)
      delete pBucketCache_;
  }
  
  /**
   * Flush trie data onto disk.
   **/
  void flush()
  {
    pBucketCache_->flush();
    pNodeCache_ ->flush();
    hashTable_.save(hash_file_+".k", hash_file_+".v");
  }

  /**
   *Load disk data into memory.
   **/
  void load()
  {
    //cout<<"loading ... \n";
    uint64_t addr = 1;
    uint32_t memAddr = 0;
    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(memAddr, addr);//root node address is 1 in disk and 0 in cache
    load_(n);
    hashTable_.load(hash_file_+".k", hash_file_+".v");
  }

public:
  /**
   *Insert a string and content address pair into trie.
   **/
  bool insert(string* pStr, const string& value)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }

    uint64_t contentAddr = addValue(value);
    if (contentAddr == (uint64_t)-1)
      return false;
    
    string consumeStr = *pStr;
    
    AlphabetNodePtr n_1 ;
    
    uint64_t addr = 1;
    uint32_t memAddr = 0;

    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(memAddr, addr);//root node address is 1 in disk and 0 in cache
        
    for (; pStr->length()>1; )
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
        pNodeCache_->unlockNode(n.getIndex());
  
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
        size_t len = pStr->length();
        b->addString(pStr, contentAddr);
        //cout<<diskAddr<<" added string!;\n";
        
        //cout<<*pBucket_;
        if (b->isFull())
        {
          //cout<<*pStr<<" ----->fullllll!"<<endl;

          //if (diskAddr == 336200)
          // cout<<"------------------ bucket full, split it*********************\n";
          // if bucket full, split it;
          vector<string> leftStr;
          vector<uint64_t> leftAddr;
          splitBucket(b, n, diskAddr, idx, leftStr, leftAddr);
          string prefix = consumeStr.substr(0, consumeStr.length()-len);

          vector<string>::iterator k=leftStr.begin();
          vector<uint64_t>::iterator v=leftAddr.begin();
          for (;k!=leftStr.end() && v!=leftAddr.end(); k++, v++)
          {
            //LDBG_<<consumeStr<<"     "<<prefix<<"    "<<(*k);
            hashTable_.insert(prefix+(*k), *v);
          }
          
        }
        
        pBucketCache_->unlockNode(memAddr);
        pNodeCache_->unlockNode(n.getIndex());
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
    
    return hashTable_.insert(consumeStr,contentAddr);
  }
  
  /**
   *Delete string in trie.
   **/
  bool del(const string& str)
  {
    return update(str, (uint64_t)-1);
  }

  /**
   *Update string 'str' related content
   **/
  bool update(const string& str, const string& value)
  {
    if (pNodeCache_==NULL)
    {
      return false;
    }

    
    uint64_t contentAddr = addValue(value);
    if (contentAddr == (uint64_t)-1)
      return false;

    uint32_t idx = 0;
    uint64_t addr = 1;
    AlphabetNodePtr n_1;
    uint32_t idx_1 = 0;
    for (size_t i=0; i<str.length()-1; i++)
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

    return hashTable_.update(str, contentAddr);
  }

  void findRegExp(const string& regexp,  vector<string>& keys, vector<string>& values)
  {
    string sofar;
    vector<item_pair> ret;
    findRegExp_(0,1, regexp, sofar, ret);

    for (typename vector<item_pair>::iterator i=ret.begin(); i!=ret.end(); i++)
    {
      keys.push_back((*i).str_);
      string v;
      readValue((*i).addr_, v);
      values.push_back(v);
    }
  }
  
  /**
   *Get the content of 'str'
   **/
  uint64_t find(const string& str, string& value)
  {
    
    if (pNodeCache_==NULL)
    {
      return -1;
    }

    
    uint64_t addr = 1;
    uint32_t rootIdx = 0;
    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(rootIdx, addr);//root node address is 1 in disk and 0 in cache
    for (size_t i=0; i<str.length()-1; i++)
    {
      //cout<<str[i]<<"====>\n";
      
      if (n.isNull())
      {
        //throw exception
        cout<<"Eception 1\n";
        return -1;
      }

      uint8_t idx = n->getIndexOf(str[i]);
      pNodeCache_->lockNode(n.getIndex());
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
        n->setMemAddr(idx, memAddr);
        pNodeCache_->unlockNode(n.getIndex());

        uint64_t address = b->getContentBy(str.substr(i));
        readValue(address,value);

        return address;
      }
      
      AlphabetNodePtr n1 = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
      n->setMemAddr(idx, memAddr);
      pNodeCache_->unlockNode(n.getIndex());
      n = n1;
    }

    uint64_t address = *(hashTable_.find(str));
    readValue(address,value);
    
    return address;
  }

  
  void display(ostream& os, const string& str)
  {
    uint32_t root = 0;
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(root, 1);
    n->display(os);
    
    for (size_t i=0; i<str.length()-1; i++)
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

    os<<"\nThen, search hash table!";
    if(*hashTable_.find(str)!= (uint64_t)-1)
    {
      os<<"Found!"<<*hashTable_.find(str)<<endl;
      string j;
      readValue(*hashTable_.find(str), j);
      os<<j<<endl;
    }
    else
      os<<"Not Found!\n";
    
  }

protected:
  void load_(AlphabetNodePtr& n)
  {
    uint64_t lastAddr = -1;
    uint32_t lastMem = -1;
    for (uint32_t i=0; i<ALPHABET_SIZE; i++)
    {
      uint64_t diskAddr = n->getDiskAddr(i);
      uint32_t memAddr = n->getMemAddr(i);

      if (lastAddr==diskAddr)
      {
        n->setMemAddr(i, lastMem);
        continue;
      }
      
      lastAddr = diskAddr;
      
      if (diskAddr == (uint64_t)-1)
        continue;
      
      if (diskAddr%2==0)
      {
        if (pBucketCache_->isFull())
        {
          continue;
        }
        
        pBucketCache_->getNodeByMemAddrForLoading(memAddr, diskAddr);
        n->setMemAddr(i, memAddr);
        lastMem = memAddr;
        continue;
      }

      if(pNodeCache_->isFull())
      {
        continue;
      }
      
      
      AlphabetNodePtr n1 = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
      n->setMemAddr(i, memAddr);
      lastMem = memAddr;
      
      load_(n1);
    }
    
  }
  
  /**
   *Split bucket indicated by idx of 'n'.
   **/
  void splitBucket(BucketPtr& b,  AlphabetNodePtr& n, uint64_t diskAddr,
                   uint8_t idx, vector<string>& leftStr, vector<uint64_t>& leftAddr)
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
    uint8_t splitPoint = b->split(bu.pN_, leftStr, leftAddr);
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

  
  /**
   *Find regular expression in trie tree.
   *@param regexp Regular expression.
   *@param sofar Record the string ahead sofar.
   *@param ret The found results.
   **/
  void findRegExp_(uint32_t idx, uint64_t addr, const string& regexp, const string& sofar,  vector<item_pair>& ret)
  {
    if (pNodeCache_==NULL)
    {
      return ;
    }

    if (regexp.empty())
    {
      uint64_t ad = hashTable_.find(sofar);
      ret.push_back(item_pair(sofar, ad));
      return;
    }
    
    
    AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(idx, addr);//root node address is 1 in disk and 0 in cache
    pNodeCache_->lockNode(n.getIndex());
          
    if (n.isNull())
    {
      //throw exception
      cout<<"Eception 1\n";
      return ;
    }
      
    string sub = regexp.substr( 1);
    uint64_t last_addr = -1;
    uint32_t last_mem = -1;
    
    switch (regexp[0])
    {
    case '*':
      for (uint32_t i=0; i<ALPHABET_SIZE; i++)
      {
        string sf = sofar;
        sf += ALPHABET[i];
        uint64_t diskAddr = n->getDiskAddr(i);
        
        if (diskAddr == last_addr)
        {
          n->setMemAddr(i, last_mem);
          continue;
        }
        
        last_addr = diskAddr;
        
        uint32_t memAddr = n->getMemAddr(i);
        if (diskAddr == (uint64_t)-1)
          continue;

        if (diskAddr%2==0)
        {
          //load bucket
          BucketPtr b = pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
          b->findRegExp(regexp, sofar, ret);
          //regexp.displayStringValue(ENCODE_TYPE, cout);

          //cout<<endl;
          
          n->setMemAddr(i, memAddr);
          last_mem = memAddr;

          continue;
        }
          
        findRegExp(memAddr, diskAddr, regexp, sf, ret);

      }
        
      findRegExp(idx, addr, sub, sofar, ret);
      pNodeCache_->unlockNode(n.getIndex());
      break;

    case '?':
      
      last_addr =  -1;
      for (uint32_t i=0; i<ALPHABET_SIZE; i++)
      {
        string sf = sofar;
        sf += ALPHABET[i];
        uint64_t diskAddr = n->getDiskAddr(i);
        uint32_t memAddr = n->getMemAddr(i);
        
        if (diskAddr == last_addr)
        {
          n->setMemAddr(i, last_mem);
          continue;
        }
        
        last_addr = diskAddr;
        
        if (diskAddr == (uint64_t)-1)
          continue;

        if (diskAddr%2==0)
        {
          //load bucket
          BucketPtr b = pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
        
          b->findRegExp(regexp, sf, ret);
          n->setMemAddr(i, memAddr);
          last_mem = memAddr;
          continue;
        }
          
        findRegExp(memAddr, diskAddr, sub, sf, ret);
      }
      findRegExp(idx, addr, sub, sofar, ret);
      pNodeCache_->unlockNode(n.getIndex());

      break;

    default:
          
      idx = n->getIndexOf(regexp[0]);
      uint64_t diskAddr = n->getDiskAddr(idx);
      uint32_t memAddr = n->getMemAddr(idx);
      string sf = sofar;
      sf += regexp[0];
      
      if (diskAddr == (uint64_t)-1)
      {
        //throw exception
        cout<<"Eception 2\n";
        return ;
      }

      
      if (diskAddr%2==0)
      {
        //load bucket
        BucketPtr b = pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
        n->setMemAddr(idx, memAddr);
        pNodeCache_->unlockNode(n.getIndex());
        //sub.displayStringValue(ENCODE_TYPE, cout);
        //b->display(cout);
        return b->findRegExp(regexp, sf, ret);
      }
      
      findRegExp(memAddr, diskAddr, sub, sf, ret);
      pNodeCache_->unlockNode(n.getIndex());
      break;
      
    }    

  }

  /**
   *Get thet total amount of nodes.
   **/
  uint32_t getNodeAmount() const
  {
    return pNodeCache_->getCount();
  }

  BucketPtr loadBucket(uint32_t& memAddr, uint64_t diskAddr)
  {
    return pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
  }

  BucketPtr newBucket()
  {
    return pBucketCache_->newNode();
    
  }

  uint64_t addValue(const string& str)
  {
    if (valuef_==NULL)
      return -1;

    fseek(valuef_, 0, SEEK_END);
    uint64_t pos = ftell(valuef_);
    if (str.compare("tsow")==0)
      cout<<pos<<endl;
    
    size_t len = str.length();
    if(fwrite(&len, sizeof(size_t), 1, valuef_)!=1)
    {
      LDBG_<<"Write length error when add value.";
      return -1;
    }

    if(fwrite(str.c_str(), len, 1, valuef_)!=1)
    {
      LDBG_<<"Write string error when add value.";
      return -1;
    }

    fflush(valuef_);
    
    return pos;    
  }
  
  bool readValue(uint64_t addr, string& str)
  {
    if (valuef_==NULL)
      return false;

    fseek(valuef_, addr, SEEK_SET);
    size_t len = 0;
    
    if(fread(&len, sizeof(size_t), 1, valuef_)!=1)
    {
      LDBG_<<"Read length error when add value.";
      return false;
    }

    char buf[len];
    if(fread(buf, len, 1, valuef_)!=1)
    {
      LDBG_<<"Write string error when add value.";
      return false;
    }

    str.assign(buf, len);
    
    return true;    
  }
  
protected:
  FILE* nodf_;//!<Node data file handler.
  FILE* bukf_;//!<Bucket data file handler.
  string hash_file_;//!<Hash table data file handler.
  FILE* valuef_;//!<Hash table data file handler.
  BucketCacheType* pBucketCache_;//!<Bucket cache.
  NodeCacheType* pNodeCache_;//!<Nodes cache
  HashTable hashTable_;//!<Hash table

}
  ;

NS_IZENELIB_AM_END
#endif

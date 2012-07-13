/**
   @file b_trie.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef STATIC_B_TRIE_HPP
#define STATIC_B_TRIE_HPP

#include <string>
#include <stdio.h>

#include <util/ustring/UString.h>

#include <am/map/map.hpp>

#include "alphabet_node.hpp"
#include "bucket.hpp"
#include "bucket_cache.hpp"
#include "node_cache.hpp"
#include "alphabet_en.hpp"
#include "alphabet_cjk.hpp"

using namespace std;

NS_IZENELIB_AM_BEGIN
/**
 *@class BTrie
 *B-trie is a kind of multi-way tree. There’re two kinds of nodes in that
 *tree. One is normal node which is an alphabet with pointer actually.
 *We just call them ’Node’. The other is called bucket which stores
 *bunches of strings suffix. The prefix is consumed by nodes. And, I
 *store nodes and bucket in 2 different separate files. The insertion of
 *a strings may involve splitting bucket.
 **/
template<
typename STRING_TYPE,
         typename STRING_TYPE::value_type* ALPHABET,
         uint32_t ALPHABET_SIZE,

         //------------bucket property-------------
         uint32_t BUCKET_SIZE = 8196,//byte
         uint8_t SPLIT_RATIO = 75,

         //--------------hash table-------------
         size_t ENTRY_SIZE_POW= 10,//2^10
         typename HASH_FUNCTION = simple_hash,
         int INIT_BTRIE_BUCKET_SIZE=64,

         //----------bucket cache---------
         uint64_t BUCKET_CACHE_LENGTH = 64*1024*1024,//bytes, it must be larger than 2 bucket size
         typename BucketCachePolicy= CachePolicyLARU,

         //----------node cache-----------
         uint64_t NODE_CACHE_LENGTH = 128*1024*1024,//bytes, it must be larger than 3 node size.
         typename NodeCachePolicy = CachePolicyLARU
         >
class BTrie
{
    typedef BTrie<STRING_TYPE, ALPHABET, ALPHABET_SIZE,
            BUCKET_SIZE, SPLIT_RATIO, ENTRY_SIZE_POW, HASH_FUNCTION,
            INIT_BTRIE_BUCKET_SIZE, BUCKET_CACHE_LENGTH, BucketCachePolicy,
            NODE_CACHE_LENGTH, NodeCachePolicy>
            SelfType;
    typedef Bucket<STRING_TYPE, BUCKET_SIZE, SPLIT_RATIO, ALPHABET, ALPHABET_SIZE> BucketType;
    typedef NodeCache<STRING_TYPE, NODE_CACHE_LENGTH, NodeCachePolicy, ALPHABET, ALPHABET_SIZE> NodeCacheType;
    typedef BucketCache<STRING_TYPE, BUCKET_CACHE_LENGTH, BUCKET_SIZE, SPLIT_RATIO, BucketCachePolicy,
            ALPHABET, ALPHABET_SIZE>
            BucketCacheType;
    typedef typename NodeCacheType::nodePtr AlphabetNodePtr;
    typedef typename BucketCacheType::nodePtr BucketPtr;
    typedef Map<string, uint64_t, ENTRY_SIZE_POW, HASH_FUNCTION, INIT_BTRIE_BUCKET_SIZE> HashMap;
    typedef typename STRING_TYPE::value_type charT;
public:
    /**
     *@param filename Name of file stores trie data. It will generate 3 files. The suffix of them are '.buk', '.nod', '.has'.
     * They stands for bucket file, trie node file and hash table file respectively.
     **/
    BTrie(const string& filename="_btrie")
        :pNodeCache_(NULL)
    {
        string bstr = filename+".buk";
        string nstr = filename + ".nod";
        hashf_ = filename + ".has";
        //valuePoolFileName_ = filename + ".val";

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

        bukf_ = fopen(bstr.c_str(), "r+");
        if (bukf_ == NULL)
        {
            bukf_ = fopen(bstr.c_str(), "w+");
        }
        pBucketCache_ =  new BucketCacheType(bukf_ );

        if (isload)
        {
            load();
        }
    }

    ~BTrie()
    {
        flush();

        if (pNodeCache_!= NULL)
            delete pNodeCache_;
        if ( pBucketCache_ != NULL)
            delete pBucketCache_;
        fclose(nodf_);
        fclose(bukf_);

        //for (vector<UString*>::iterator i=strPool_.begin(); i!=strPool_.end(); i++)
        //delete *i;
    }

    /**
     * Flush trie data onto disk.
     **/
    void flush()
    {
        pBucketCache_->flush();
        pNodeCache_ ->flush();
        hashTable_.save(hashf_);

//     ofstream of(valuePoolFileName.c_str());
//     oarchive oa(of);
//     size_t size = dataVec_.size();
//     oa << size;

//     for(typename vector<ValueType>::iterator i =dataVec_.begin(); i!=dataVec_.end(); i++)
//     {
//       oa<<(*i);
//     }

//     of.close();

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

        hashTable_.load(hashf_);


//       ifstream ifs(valuePoolFileName_.c_str());
//       iarchive ia(ifs);
//       size_t size;
//       ia>>size;

//       for (size_t i =0; i<size; i++)
//       {
//         ValueType v;
//         ia>>v;
//         //cout<<v;
//         dataVec_.push_back(v);
//       }

//       ifs.close();
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

    string substr(const string& str, size_t pos, size_t len = (size_t)-1)
    {
        if (len != (size_t)-1)
            return str.substr(pos, len);

        return str.substr(pos);
    }

    izenelib::util::UString substr(const izenelib::util::UString& str, size_t pos, size_t len=(size_t)-1)
    {
        izenelib::util::UString tmp;
        if (len != (size_t)-1)
        {
            str.substr(tmp, pos, len);
            return tmp;
        }

        str.substr(tmp, pos);
        return tmp;
    }

public:
    /**
     *Insert a string and content address pair into trie.
     **/
    bool insert(const STRING_TYPE& str, uint64_t contentAddr)
    {
        if (pNodeCache_==NULL)
        {
            return false;
        }

        STRING_TYPE* pStr = new STRING_TYPE(str);

        AlphabetNodePtr n_1 ;

        uint64_t addr = 1;
        uint32_t memAddr = 0;

        AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(memAddr, addr);//root node address is 1 in disk and 0 in cache

        for (; pStr->length()>1; )
        {
            if (n.isNull())
            {
                //throw exception
                delete pStr;
                return false;
            }

            pNodeCache_->lockNode(n.getIndex());
            uint32_t idx = n->getIndexOf((*pStr)[0]);
            if(idx == (uint32_t)-1)
                std::cout << "err1"<<endl;
            uint64_t diskAddr = n->getDiskAddr(idx);
            memAddr = n->getMemAddr(idx);

            if(diskAddr == (uint64_t)-1 )
            {
                // new bucket, then add rest string into bucket, return.
                //cout<<"new bucket, then add rest string into bucket, return.\n";

                BucketPtr b = newBucket();

                b->addString(pStr, contentAddr);
                uint64_t addr =b->add2disk();
                n->setAllDiskAddr( addr);
                n->setAllMemAddr(b.getIndex());

                return true;
            }


            if (diskAddr%2==0)
            {
                //load bucket

                //cout<<diskAddr<<" load bucket;\n";

                BucketPtr b = loadBucket(memAddr, diskAddr);
                pBucketCache_->lockNode(memAddr);

                //b->display(cout);

                n->setMemAddr(idx, memAddr);

                size_t len = pStr->length();
                b->addString(pStr, contentAddr);
                //cout<<diskAddr<<" added string!;\n";

                //cout<<*pBucket_;
                if (b->isFull())
                {
                    //cout<<" ----->fullllll!"<<endl;

                    vector<STRING_TYPE> leftStr;
                    vector<uint64_t> leftAddr;
                    splitBucket(b, n, diskAddr, idx, leftStr, leftAddr);
                    STRING_TYPE prefix = substr(str, 0, str.length()-len);
                    //consumeStr.subString(prefix,0, consumeStr.length()-len);

                    typename vector<STRING_TYPE>::iterator k=leftStr.begin();
                    vector<uint64_t>::iterator v=leftAddr.begin();
                    for (; k!=leftStr.end()&& v!=leftAddr.end(); v++,k++)
                    {
                        STRING_TYPE tmp = prefix;
                        tmp += (*k);

                        string ss((char*)tmp.c_str(), tmp.size());
                        //tmp.convertString(ss, UString::UTF_8);
                        //ss += '\0';
                        //cout<<ss.size()<<"hash table\n";
                        hashTable_.insert(ss, *v);
                    }

                    //cout<<"splitted...\n";
                }

                //n->display(cout);

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

            *pStr = substr(*pStr ,1);
        }

        delete pStr;

        string ss((char*)str.c_str(), str.size());
        //ss += '\0';
        //delete pStr;
        //str.convertString(ss, UString::UTF_8);
        //cout<<ss.size()<<"hash table\n";
        return hashTable_.insert(ss,contentAddr);
    }

    /**
     *Split bucket indicated by idx of 'n'.
     **/
    void splitBucket(BucketPtr& b,  AlphabetNodePtr& n, uint64_t diskAddr,
                     uint32_t idx, vector<STRING_TYPE>& leftStr, vector<uint64_t>& leftAddr)
    {
        charT up = b->getUpBound();
        charT low = b->getLowBound();
//    cout<<up<<" "<<low<<"  "<<b->getStrGroupAmount()<<" 8888888888\n";

        if (b->getStrGroupAmount()==1 && up!=low)
        {
            //if only have one string group
            charT splitPoint = b->getGroupChar(0);
//      cout<<splitPoint<<"  00000000000\n";
            if (up!=splitPoint)
            {
                charT beforeSplitPoint = ALPHABET[n->getIndexOf(splitPoint)-1];
                BucketPtr b1 =  pBucketCache_->newNode();

                b1->setUpBound(up);
                b1->setLowBound(beforeSplitPoint);
                n->setDiskAddr(up, beforeSplitPoint, b1->add2disk());
                n->setMemAddr(up, beforeSplitPoint, b1.getIndex());
            }

            if (splitPoint != low)
            {
                charT afterSplitPoint = ALPHABET[n->getIndexOf(splitPoint)+1];
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
//    cout<<"splitting!\n";
        charT splitPoint = b->split(bu.pN_, leftStr, leftAddr);

        AlphabetNodePtr parent = n;
        while(splitPoint == -1 && b->getStrGroupAmount()==1)
        {
            AlphabetNodePtr newNode = pNodeCache_->newNode();
//      std::cout << "insert middle alphabet node" << std::endl;
            newNode->setDiskAddr(ALPHABET[0], ALPHABET[ALPHABET_SIZE-1], diskAddr);
            newNode->setMemAddr(ALPHABET[0], ALPHABET[ALPHABET_SIZE-1], b.getIndex());

            charT sp = b->getGroupChar(0);
            if (ALPHABET[0]!=sp)
            {
                charT bsp = ALPHABET[n->getIndexOf(sp)-1];
                BucketPtr b1 =  pBucketCache_->newNode();

                b1->setUpBound(ALPHABET[0]);
                b1->setLowBound(bsp);
                n->setDiskAddr(ALPHABET[0], bsp, b1->add2disk());
                n->setMemAddr(ALPHABET[0], bsp, b1.getIndex());
            }

            if (ALPHABET[ALPHABET_SIZE-1] != sp)
            {
                charT asp = ALPHABET[n->getIndexOf(sp)+1];
                BucketPtr b2  = pBucketCache_->newNode();

                b2->setUpBound(asp);
                b2->setLowBound(ALPHABET[ALPHABET_SIZE-1]);
                n->setDiskAddr(asp, ALPHABET[ALPHABET_SIZE-1], b2->add2disk());
                n->setMemAddr(asp, ALPHABET[ALPHABET_SIZE-1], b2.getIndex());
            }
            b->setUpBound(sp);
            b->setLowBound(sp);

            parent->setDiskAddr(idx, newNode->add2disk());
            parent->setMemAddr(idx, newNode.getIndex());
            parent = newNode;
            splitPoint = b->split(bu.pN_, leftStr, leftAddr);
//      cout<<"splitting point: "<<splitPoint<<endl;
        }

        if(splitPoint == -1)
            throw std::runtime_error("bad splitPoint");


        uint64_t newBktAddr =  bu->add2disk();

        if (up == low)
        {
            //cout<<pNodeCache_->getCount();
            //cout<<"pBucket_->isPure() \n";

            AlphabetNodePtr newNode = pNodeCache_->newNode();
//      std::cout << hex << (int)ALPHABET[0] << "," << (int)ALPHABET[ALPHABET_SIZE-1] << dec << "," << ALPHABET_SIZE << std::endl;
//      std::cout << hex << (int)splitPoint << "," << (int)bu->getUpBound() << std::endl;
            newNode->setDiskAddr(ALPHABET[0], splitPoint, diskAddr);
            newNode->setDiskAddr(bu->getUpBound(), ALPHABET[ALPHABET_SIZE-1], newBktAddr);

            newNode->setMemAddr(ALPHABET[0], splitPoint, b.getIndex());
            newNode->setMemAddr(bu->getUpBound(), ALPHABET[ALPHABET_SIZE-1], bu.getIndex());

            parent->setDiskAddr(idx, newNode->add2disk());
            parent->setMemAddr(idx, newNode.getIndex());
        }
        else
        {
            //cout<<"pBucket_ is not Pure\n";
            parent->setDiskAddr(up, splitPoint, diskAddr);
            parent->setMemAddr(up, splitPoint, b.getIndex());
            parent->setDiskAddr(bu->getUpBound(), low, newBktAddr);
            parent->setMemAddr(bu->getUpBound(), low, bu.getIndex());
        }

    }

    /**
     *Delete string in trie.
     **/
    bool del(const STRING_TYPE& str)
    {
        return update(str, (uint64_t)-1);
    }

    /**
     *Update string 'str' related content
     **/
    bool update(const STRING_TYPE& str, uint64_t contentAddr)
    {
        if (pNodeCache_==NULL)
        {
            return false;
        }

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

                //UString tmp;
                //str.subString(tmp, i-1);

                b->updateContent(substr(str, i-1),contentAddr);
                return b->update2disk();
            }

            AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(idx, addr);

            if (n_1!=NULL)
            {
                n_1->setMemAddr(idx_1, idx);
            }
            idx_1 = idx;
            n_1 = n;

            uint32_t ch = n->getIndexOf(str[i]);
            idx = n->getMemAddr(ch);
            addr = n->getDiskAddr(ch);
        }

        string ss((char*)str.c_str(), str.size());
        return hashTable_.update(ss, contentAddr);
    }

    bool findRegExp(const STRING_TYPE& regexp,  vector<uint32_t>& ret)
    {
        vector<item_pair<STRING_TYPE> > v;
        findRegExp(regexp, v);

        for (typename vector<item_pair<STRING_TYPE> >::iterator i=v.begin(); i!=v.end(); i++)
            ret.push_back((uint32_t)i->addr_);

        return ret.size()!=0;
    }

    bool findRegExp(const STRING_TYPE& regexp,  vector<item_pair<STRING_TYPE> >& ret)
    {
        STRING_TYPE sofar;
        findRegExp_(0,1, regexp, sofar, ret);
        return ret.size()!=0;
    }


    /**
     *Get the content of 'str'
     **/
    uint64_t find(const STRING_TYPE& str)
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
            //cout<<str[i]<<"====>\n";

            if (n.isNull())
            {
                //throw exception
                cout<<"Eception 1\n";
                return -1;
            }

            uint32_t idx = n->getIndexOf(str[i]);
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
                //b->display(cout);
                //cout<<str.substr(i)<<"++++++>";

                //UString tmp;
                //str.subString(tmp, i);
                n->setMemAddr(idx, memAddr);
                pNodeCache_->unlockNode(n.getIndex());
                return b->getContentBy(substr(str, i));
            }

            AlphabetNodePtr n1 = pNodeCache_->getNodeByMemAddr(memAddr, diskAddr);
            n->setMemAddr(idx, memAddr);
            pNodeCache_->unlockNode(n.getIndex());
            n = n1;
        }

        string ss((char*)str.c_str(), str.size());
        //cout<<"find from hashtable\n";

        //str.convertString(ss, UString::UTF_8);
        uint64_t* t = hashTable_.find(ss);
        if (t ==NULL)
            return -1;

        return *t;
    }

    /**
     *Get thet total amount of nodes.
     **/
    uint32_t getNodeAmount() const
    {
        return pNodeCache_->getCount();
    }

    void display(ostream& os, const STRING_TYPE& str)
    {
        uint32_t root = 0;
        AlphabetNodePtr n = pNodeCache_->getNodeByMemAddr(root, 1);
        //n->display(os);

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
        string ss((char*)str.c_str(), str.size());
        if(hashTable_.find(ss)!= (uint64_t)-1)
            os<<"Found!"<<endl;
        else
            os<<"Not Found!\n";

    }

    BucketPtr loadBucket(uint32_t& memAddr, uint64_t diskAddr)
    {
        return pBucketCache_->getNodeByMemAddr(memAddr, diskAddr);
    }

    BucketPtr newBucket()
    {
        return pBucketCache_->newNode();

    }

protected:

    /**
     *Find regular expression in trie tree.
     *@param regexp Regular expression.
     *@param sofar Record the string ahead sofar.
     *@param ret The found results.
     **/
    void findRegExp_(uint32_t idx, uint64_t addr, const STRING_TYPE& regexp, const STRING_TYPE& sofar,  vector<item_pair<STRING_TYPE> >& ret)
    {

        if (pNodeCache_==NULL)
        {
            return ;
        }

        if (regexp.empty())
        {
            string ss((char*)sofar.c_str(), sofar.size());
            uint64_t ad = *(hashTable_.find(ss));
            ret.push_back(item_pair<STRING_TYPE>(sofar, ad));
            //ret.push_back(ad);
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

        STRING_TYPE sub = substr(regexp , 1);
        //regexp.subString(sub, 1);
        uint64_t last_addr = -1;
        uint32_t last_mem = -1;

        switch (regexp[0])
        {
        case '*':
            for (uint32_t i=0; i<ALPHABET_SIZE; i++)
            {
                STRING_TYPE sf = sofar;
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

                findRegExp_(memAddr, diskAddr, regexp, sf, ret);

            }

            findRegExp_(idx, addr, sub, sofar, ret);
            pNodeCache_->unlockNode(n.getIndex());
            break;

        case '?':

            last_addr =  -1;
            for (uint32_t i=0; i<ALPHABET_SIZE; i++)
            {
                STRING_TYPE sf = sofar;
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

                findRegExp_(memAddr, diskAddr, sub, sf, ret);
            }
            findRegExp_(idx, addr, sub, sofar, ret);
            pNodeCache_->unlockNode(n.getIndex());

            break;

        default:

            idx = n->getIndexOf(regexp[0]);
            uint64_t diskAddr = n->getDiskAddr(idx);
            uint32_t memAddr = n->getMemAddr(idx);
            STRING_TYPE sf = sofar;
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

            findRegExp_(memAddr, diskAddr, sub, sf, ret);
            pNodeCache_->unlockNode(n.getIndex());
            break;

        }
    }

protected:
    FILE* nodf_;//!<Node data file handler.
    FILE* bukf_;//!<Bucket data file handler.
    string hashf_;//!<Hash table data file handler.
    BucketCacheType* pBucketCache_;//!<Bucket cache.
    NodeCacheType* pNodeCache_;//!<Nodes cache
    HashMap hashTable_;//!<Hash table
    //  string valuePoolFileName_;
    //vector<ValueType> valuePool_;
};

typedef BTrie<std::string, en, en_size> BTrie_En;

typedef BTrie<izenelib::util::UString, cjk, cjk_size> BTrie_CJK;

NS_IZENELIB_AM_END
#endif

/**
   @file node_cache.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef NODE_CACHE_HPP
#define NODE_CACHE_HPP

#include <stdio.h>
#include "alphabet_node.hpp"
#include <time.h>
#include "cache_strategy.h"
#include <string>

NS_IZENELIB_AM_BEGIN
using namespace std;


/**
 *@class NodeCache
 **/
template<
class STRING_TYPE,
      uint64_t CACHE_LENGTH,//bytes
      class CacheType,
      typename STRING_TYPE::value_type* ALPHABET,
      uint32_t ALPHABET_SIZE
      >
class NodeCache
{
public:
    typedef AlphabetNode<typename STRING_TYPE::value_type, ALPHABET, ALPHABET_SIZE> NodeType;

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
    typedef NodeCache <STRING_TYPE, CACHE_LENGTH, CacheType, ALPHABET, ALPHABET_SIZE> SelfType;

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

    /**
     *@param f Node data file handler.
     *@param rootAddr The trie tree root node's disk address
     **/
    NodeCache(FILE* f, uint64_t rootAddr)
        :f_(f),rootAddr_(rootAddr),count_(0)
    {
        nodes  = new struct _cache_node_ [CACHE_SIZE];
    }

    ~NodeCache()
    {
        for (uint32_t i=0; i<CACHE_SIZE; i++)
        {
            if (nodes[i].pNode_ != NULL)
            {
                delete nodes[i].pNode_;
                nodes[i].pNode_ = NULL;
            }
        }
        delete [] nodes;
    }

    /**
     *Load data from disk.
     **/
    uint32_t load()
    {
        vector<uint32_t> indexes;
        indexes.push_back(0);


        NodeType* t = new NodeType(f_);
        t->load(rootAddr_);
        nodes[count_] = _cache_node_(t);
        count_++;

        while (indexes.size()>0)
        {
            load_(indexes);
        }

        return count_;
    }

    void load_(vector<uint32_t>& indexes)
    {
        uint32_t idx = indexes.front();
        indexes.erase(indexes.begin());

        NodeType* n = nodes[idx].pNode_ ;
        nodes[idx] =  _cache_node_(n);

        for (uint32_t i=0; i<n->getSize(); i++)
        {
            if (n->getDiskAddr(i)==(uint64_t)-1)
                continue;

            if (n->getDiskAddr(i)%2==0)
                continue;

            NodeType* t = new NodeType(f_);
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

    /**
     *Re-load data from file.
     **/
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


    /*!
     *Find out the switched one.
     **/
    uint32_t findSwitchOut() const
    {
        uint32_t minIdx = 1;
        while(minIdx<count_ && nodes[minIdx].locked_)
            minIdx++;
        if (minIdx==count_)
            cout<<"All locked up, Why?\n";

        for (uint32_t i=count_-1; i>0; i--)
        {
            if (nodes[i].pNode_==NULL)
                return i;

            if (!nodes[i].locked_ && nodes[i].cacheInfo_.compare(nodes[minIdx].cacheInfo_)<0)
            {
                minIdx = i;
            }

        }

        return minIdx;
    }

    /*!
     *Lock mode in order to not be switched out.
     **/
    void lockNode(uint32_t memAddr)
    {
        nodes[memAddr].locked_ = true;
    }

    /*!
     *Unclock node.
     **/
    void unlockNode(uint32_t memAddr)
    {
        nodes[memAddr].locked_ = false;
    }

    /*!
     *Get node count.
     **/
    uint32_t getCount()const
    {
        return count_;
    }

    /*!
     *Get node by specific disk address into memory.
     **/
    nodePtr getNodeByMemAddr(uint32_t& memAddr, uint64_t diskAddr)
    {
        if (diskAddr%2==0)
            return nodePtr();

        if (memAddr>=count_ && memAddr!=(uint32_t)-1 && count_!=0)
        {
            return nodePtr();
        }

        if (memAddr!=(uint32_t)-1)
        {
            NodeType* t = nodes[memAddr].pNode_;
            if (t!=NULL && t->getDiskAddr()==diskAddr)
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
        // cout<<"\nswitch out-: "<<memAddr<<endl;
        kickOutNodes(memAddr);


        NodeType* t = new NodeType(f_);
        t->load(diskAddr);
        nodes[memAddr] = _cache_node_(t);
        return nodePtr(nodes[memAddr], memAddr);
    }


    /*!
     *Get a new node in cache which is stroed in position 'diskAddr'.
     **/
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

    /*!
     *New a node
     **/
    nodePtr newNode()
    {
        //cout<<"newNode "<<CACHE_SIZE<<endl;

        if (count_<CACHE_SIZE)
        {
            NodeType* t = new NodeType(f_);
            nodes[count_] = _cache_node_(t);
            nodePtr p(nodes[count_], count_);
            count_++;
            return p;
        }


        uint32_t ret = findSwitchOut();
        //cout<<"\nswitch out: "<<ret<<endl;
        kickOutNodes(ret);

        NodeType* t = new NodeType(f_);
        nodes[ret] = _cache_node_(t);
        nodePtr p(nodes[ret], ret);
        return p;

    }


    /*!
     *Kick out the 'memAddr' node.
     **/
    uint64_t kickOutNodes(uint32_t memAddr)
    {
        // cout<<"KickOutNodes(): ";
//     cout<<count_<<"  "<<CACHE_SIZE<<"  "<<memAddr<<endl;

        if (memAddr==(uint32_t)-1 || nodes[memAddr].pNode_ == NULL)
            return (uint64_t)-1;

        uint64_t last_disk = -1;
        for (uint32_t i=0; i<nodes[memAddr].pNode_->getSize(); i++)
        {
            uint32_t m = nodes[memAddr].pNode_->getMemAddr(i);
            uint64_t d = nodes[memAddr].pNode_->getDiskAddr(i);
            //    cout<<memAddr<<"    "<<m<<"   "<<d<<"pppp\n";

            if (d==(uint64_t)-1 || d==last_disk || m == (uint32_t)-1
                    || nodes[m].pNode_==NULL || d%2==0 || nodes[m].pNode_->getDiskAddr()!=d )
            {
                last_disk = d;
                continue;
            }


            last_disk = d;
            if (m==memAddr)
                cout<<i<<" kick out sub nodes: "<<m<<"  "<<d<<endl;

            kickOutNodes(m);
            //nodes[memAddr].pNode_->setDiskAddr(i, 2);
            //nodes[memAddr].pNode_->setDiskAddr(i, kickOutNodes(m));
        }

        uint64_t ret = nodes[memAddr].pNode_->update2disk();
        delete nodes[memAddr].pNode_;
        nodes[memAddr].pNode_ = NULL;
        nodes[memAddr]=  _cache_node_();

        return ret;

    }

    /*!
     *Flush nodes data into disk.
     **/
    void flush()
    {
        for (uint32_t i=0; i<CACHE_SIZE; i++)
            if (nodes[i].pNode_ != NULL)
                nodes[i].pNode_->update2disk();

        fflush(f_);
    }


    bool isFull()
    {
        return count_>=CACHE_SIZE;
    }

protected:
    FILE* f_;//!<File handler
    uint32_t rootAddr_;//!<Trie tree root node disk address
    uint32_t count_;//!< Node count
    struct _cache_node_* nodes;//!<Nods are stored in this array.

}
;


NS_IZENELIB_AM_END
#endif

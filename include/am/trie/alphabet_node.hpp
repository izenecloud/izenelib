/**
   @file alphabet_node.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef ALPHABET_NODE_HPP
#define ALPHABET_NODE_HPP

#include <stdio.h>
#include <types.h>
#include <iostream>
#include "alphabet.hpp"
#include <util/log.h>

#include <execinfo.h>
#include <stdlib.h>

using namespace std;

NS_IZENELIB_AM_BEGIN

/**
 * @class AlphabetNode
 *B-tire has two types of node, alphabet node and bucket. This is for laphabet node.
 **/
template<
class CHAR_T,
      CHAR_T* ALPHABET,
      uint32_t ALPHABET_SIZE
      >
class AlphabetNode
{

    struct _disk_node_
    {
        uint64_t addrs_[ALPHABET_SIZE]  ;
        uint32_t level_;

    }
    ;

    struct _node_
    {
        struct _disk_node_ diskNode_;
        uint32_t mem_addr_[ALPHABET_SIZE];
        bool dirty_;
        uint64_t diskPos_;

        inline _node_()
        {
            for (uint32_t i =0; i<ALPHABET_SIZE; i++)
            {
                mem_addr_[i] = (uint32_t)-1;
                diskNode_.addrs_[i] = (uint64_t)-1;
            }
            dirty_ = true;
            diskPos_ = (uint64_t)-1;
            diskNode_.level_ = (uint32_t)-1;

        }

    };

public:
    typedef AlphabetNode<CHAR_T, ALPHABET, ALPHABET_SIZE> SelfType;
    enum slef_size { SIZE_= sizeof(_node_)+sizeof(_node_*)+sizeof(FILE*)};

    /**
     * @param f Used for storing alphabet nodes data
     **/
    AlphabetNode(FILE* f)
        :f_(f), pNode_(NULL)
    {
        pNode_ = new struct _node_();
    }

    ~AlphabetNode()
    {
        if (pNode_!=NULL)
            delete pNode_;
        pNode_ = NULL;
    }


    /**
     *Input the node to an ostream.
     **/
    ostream& display(ostream& os)
    {
        os<<"\ndis: [";
        for (uint32_t i=0; i<ALPHABET_SIZE; i++)
        {
            os<<pNode_->diskNode_.addrs_[i]<<",";
        }
        os<<"]\n";

        os<<"\nmem: [";
        for (uint32_t i=0; i<ALPHABET_SIZE; i++)
        {
            os<<pNode_->mem_addr_[i]<<",";
        }
        os<<"]\n";

        os<<"level:"<<pNode_->diskNode_.level_<<" dirty:"<<pNode_->dirty_<<" disk:"<<pNode_->diskPos_<<endl;

        return os;
    }


    /**
     *Input the node to an ostream.
     **/
    friend ostream& operator << ( ostream& os, const SelfType& node)
    {
        os<<"\ndis: [";
        for (uint32_t i=0; i<ALPHABET_SIZE; i++)
        {
            os<<node.pNode_->diskNode_.addrs_[i]<<",";
        }
        os<<"]\n";

        os<<"\nmem: [";
        for (uint32_t i=0; i<ALPHABET_SIZE; i++)
        {
            os<<node.pNode_->mem_addr_[i]<<",";
        }
        os<<"]\n";

        os<<"level:"<<node.pNode_->diskNode_.level_<<" dirty:"<<node.pNode_->dirty_<<" disk:"<<node.pNode_->diskPos_<<endl;

        return os;
    }


    /**
     *Load a node from disk.
     *@param addr Indicate the start position of an node in disk.
     **/
    bool load(uint64_t addr)
    {
        if (pNode_!=NULL)
        {
            update2disk();
            delete pNode_;
        }

        pNode_ = NULL;

        if (addr == (uint64_t)-1)
            return false;


        pNode_ = new struct _node_();
        if (fseek(f_, addr, SEEK_SET)!=0)
        {
            //throw exception
            return false;
        }

        if(fread(&(pNode_->diskNode_), sizeof(struct _disk_node_), 1, f_)!=1)
        {
            ///throw exception
            return false;
        }


        pNode_->dirty_ = false;
        pNode_->diskPos_ = addr;
        for (uint32_t i =0; i<ALPHABET_SIZE; i++)
            pNode_->mem_addr_[i] = (uint32_t)-1;

        return true;
    }

    /**
     *Update the disk node data by node in memory
     *
     **/
    uint64_t update2disk()
    {
        if (pNode_==NULL)
            return false;

        if (pNode_->dirty_)
        {
            pNode_->dirty_ = false;
            if (pNode_->diskPos_ == (uint64_t)-1)
            {
                return add2disk();
            }

            fseek(f_, pNode_->diskPos_, SEEK_SET);
            if ( fwrite(&(pNode_->diskNode_), sizeof(struct _disk_node_), 1, f_)!=1)
            {
                return (uint64_t)-1;
            }
            return pNode_->diskPos_;
        }

        return true;
    }

    /**
     *Add a new node to the tail of the node data file.
     **/
    uint64_t add2disk()
    {
        if (pNode_==NULL)
        {
            return (uint64_t)-1;
        }

        fseek(f_, 0, SEEK_END);
        uint64_t end = ftell(f_);


        if (end%2==0)
        {
            //alphabet node only be stored at odds address, which distinguish from bucket
            end = fseek(f_, 1, SEEK_END);
            end++;
        }


        if (fwrite(&(pNode_->diskNode_), sizeof(struct _disk_node_), 1, f_)!=1)
            return (uint64_t)-1;

        pNode_->dirty_ = 1;
        pNode_->diskPos_ = end;
        return end;

    }

    /**
     *Get the index of an char in the alphabet
     **/
    static uint32_t getIndexOf(CHAR_T ch)
    {
        if (ch<ALPHABET[0] || ch>ALPHABET[ALPHABET_SIZE-1])
        {
            LDBG_<<"Can't find '0x"<<hex<< (int)ch<<dec<<"' in alphabet, which is out of range ["
                 <<ALPHABET[0]<<","<<ALPHABET[ALPHABET_SIZE-1]<<"]"<<endl;
            return -1;
        }

        if(ALPHABET[0] == 'a' && ALPHABET[ALPHABET_SIZE-1]=='z')
        {
            if (ch <= 'Z' && ch >='A')
                return ch-'A';
            return ch-ALPHABET[0];
        }

        uint32_t start = 0;
        uint32_t end  = ALPHABET_SIZE -1;
        uint32_t mid = (start + end)/2;

        while ( mid<=end && mid>=start)
        {
            if (ALPHABET[mid]==ch)
                return mid;

            if (ALPHABET[mid]<ch)
            {
                start = mid+1;
                mid = (start + end)/2;
                continue;
            }

            if (ALPHABET[mid]>ch)
            {
                end = mid-1;
                mid = (start + end)/2;
                continue;
            }
        }

        return -1;
    }

    /**
     *Get aphabet size.
     **/
    uint32_t getSize() const
    {
        return ALPHABET_SIZE;
    }

    /**
     *Set the next node memory address of the one char in alphabet.
     **/
    void setMemAddr(uint32_t index,uint32_t addr )
    {
        if (index>=ALPHABET_SIZE)
        {
            //throw exception
            return;
        }

        if (pNode_ == NULL)
        {
            cout<<"ERRor: alphabet node -> setMemAddr()\n";
        }

        pNode_->mem_addr_[index] = addr;
    }


    /**
     *Set the next node disk address of the one char in alphabet.
     **/
    void setDiskAddr(uint32_t index,uint64_t addr )
    {
        if (index>=ALPHABET_SIZE)
        {
            //throw exception
            return;
        }
        if (pNode_->diskNode_.addrs_[index] == addr)
            return;

        pNode_->dirty_ = true;
        pNode_->diskNode_.addrs_[index] = addr;
    }

    /**
     *Set memory adresses from 'fromCh' to 'toCh' the same address.
     **/
    void setMemAddr(CHAR_T fromCh, CHAR_T toCh, uint32_t addr )
    {

        for (uint32_t i= getIndexOf(fromCh); i<=getIndexOf(toCh); i++)
            if (pNode_->mem_addr_[i] != addr)
            {
                pNode_->dirty_ = true;
                pNode_->mem_addr_[i] = addr;
            }
    }

    /**
     *Set disk adresses from 'fromCh' to 'toCh' the same address.
     **/
    void setDiskAddr(CHAR_T fromCh, CHAR_T toCh, uint64_t addr )
    {
        for (uint32_t i= getIndexOf(fromCh); i<=getIndexOf(toCh); i++)
            if (pNode_->diskNode_.addrs_[i] != addr)
            {
                pNode_->dirty_ = true;
                pNode_->diskNode_.addrs_[i] = addr;
            }
    }

    /**
     *Set all the char one next-node disk address
     **/
    void setAllDiskAddr(uint64_t addr)
    {
        for (uint32_t i= 0; i<getSize(); i++)
            if (pNode_->diskNode_.addrs_[i] != addr)
            {
                pNode_->dirty_ = true;
                pNode_->diskNode_.addrs_[i] = addr;
            }
    }


    /**
     *Set all the char one next-node memory address
     **/
    void setAllMemAddr(uint64_t addr)
    {
        for (uint32_t i= 0; i<getSize(); i++)
            if (pNode_->mem_addr_[i] != addr)
            {
                pNode_->dirty_ = true;
                pNode_->mem_addr_[i] = addr;
            }
    }

    /**
     *Get memory address of 'index' in alphabet.
     **/
    uint32_t getMemAddr(uint32_t index) const
    {
        return pNode_->mem_addr_[index];
    }


    /**
     *Get disk address of 'index' in alphabet.
     **/
    uint64_t getDiskAddr(uint32_t index) const
    {
        return pNode_->diskNode_.addrs_[index];
    }

    /**
     *Get self's disk adress.
     **/
    uint64_t getDiskAddr() const
    {
        return pNode_->diskPos_;
    }



protected:
    FILE* f_;//!< File handler for node data file.
    struct _node_* pNode_;//!< Data storage
}
;



NS_IZENELIB_AM_END
#endif

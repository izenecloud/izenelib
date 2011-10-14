#ifndef SKIPNODE_H_
#define SKIPNODE_H_

#include "slf_types.h"
#include "SlfHeader.h"

NS_IZENELIB_AM_BEGIN
/**
 * @brief SkipNode reprents a node in a SkipList.
 */

template <typename DataType, typename LockType, typename Alloc> class SkipNode
{

public:
    SkipNode(const DataType& theElement = DataType(), int h = 0) :
            element(theElement), height(h)
    {
        for (int i=0; i<MAX_LEVEL; i++)
        {
            right[i] = 0;
            rfpos[i] = 0;
        }
        isLoaded = false;
        isDirty = true;
        activeNodeNum++;
    }
    virtual ~SkipNode()
    {
        isLoaded = false;
        isDirty = false;
        activeNodeNum--;
    }

    void unload()
    {
        //to do
        isLoaded = false;
    }

    bool read(FILE* f);
    bool write(FILE* f);

    inline bool read(char* pBuf);
    inline bool write(char* pBuf);

    inline SkipNode<DataType, LockType, Alloc>* loadRight(int h, FILE* f)
    {
        //display();
        //cout<<"!!!"<<h<<endl;
        if ( right[h] && !right[h]->isLoaded )
        {
            //cout<<"!!! rrr"<<h<<endl;
            right[h]->read(f);
        }
        //	if(right[h])right[h]->left[h] = this;
        if (right[h])cout<<" -> "<<element.get_key();
        return right[h];
    }

    void display(std::ostream& os = std::cout)
    {
        os<<"diplaying skipnode...\n\n";
        os<<element.get_key()<<endl;
        os<<"isLoaded: "<<isLoaded<<endl;
        os<<"isDirty: "<<isDirty<<endl;
        os<<"fpos: "<<fpos<<endl;
        os<<"height: "<<height<<endl;
        for (int i=0; i<height; i++)
        {
            os<<"rfpos: "<<rfpos[i]<<endl;
        }

    }

    static void setDataSize(size_t maxDataSize, size_t pageSize)
    {
        maxDataSize_ = maxDataSize;
        pageSize_ = pageSize;
    }
public:
    DataType element;
    SkipNode* right[MAX_LEVEL];
    //SkipNode* left[MAX_LEVEL];
    long rfpos[MAX_LEVEL];
    NodeID nid;

    int height;
public:

    bool isLoaded;
    bool isDirty;
    long fpos;

    static size_t activeNodeNum;
private:
    DbObjPtr object_;
    static size_t maxDataSize_;
    static size_t pageSize_;
    static LockType lock_;
    static map<long, SkipNode<DataType, LockType, Alloc>*> pos2node_;
};

template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
DataType, LockType, Alloc>::activeNodeNum;

template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
DataType, LockType, Alloc>::maxDataSize_;

template<typename DataType, typename LockType, typename Alloc> size_t SkipNode<
DataType, LockType, Alloc>::pageSize_;

template<typename DataType, typename LockType, typename Alloc> LockType
SkipNode<DataType, LockType, Alloc>::lock_;

template<typename DataType, typename LockType, typename Alloc> map<long, SkipNode<DataType, LockType, Alloc>*>
SkipNode<DataType, LockType, Alloc>::pos2node_;

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
DataType, LockType, Alloc>::read(FILE *f)
{

    int n = (fpos - sizeof(SlfHeader) )/pageSize_;
    n /= BLOCK_SIZE;

    cout<<n<<endl;
    if ( !mbList[n])
    {
        mbList[n] = new MemBlock(sizeof(SlfHeader) + n*pageSize_*BLOCK_SIZE, pageSize_);
        mbList[n]-> load(f);
    }
    return read( mbList[n]->getP(fpos) );
}

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
DataType, LockType, Alloc>::read(char *pBuf)
{
    char *p = pBuf;
    size_t recSize;

    char* temp;

    if (!p)
    {
        return false;
    }

    memcpy(&height, p, sizeof(int));
    p += sizeof(int);
    memcpy(&rfpos[0], p, sizeof(long)*MAX_LEVEL);
    p += sizeof(long)*MAX_LEVEL;

    for (int i=0; i<height; i++)
    {
        if (rfpos[i] != 0)
        {
            if ( right[i] == 0 )
            {
                if ( !pos2node_[rfpos[i]] )
                {
                    right[i] = new SkipNode<DataType, LockType, Alloc>;
                    right[i]->fpos = rfpos[i];
                    pos2node_[rfpos[i]] = right[i];
                }
                else
                {
                    right[i] = pos2node_[rfpos[i]];
                }
            }
        }
    }

    memcpy(&recSize, p, sizeof(size_t));
    p += sizeof(size_t);

    temp = new char[recSize];
    if (recSize <= maxDataSize_)
    {
        memcpy(temp, p, recSize);
        p += recSize;
    }
    else
    {
        assert(false);
    }

    assert(recSize != 0);
    object_.reset(new DbObj(temp, recSize));
    read_image(element, object_);

    delete[] temp;

    isLoaded = true;
    isDirty = false;
    return true;

}

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
DataType, LockType, Alloc>::write(FILE *f)
{

    if (!isDirty)
    {
        return true;
    }

    int n = (fpos - sizeof(SlfHeader) )/(pageSize_*BLOCK_SIZE);
    if ( !mbList[n])
    {
        mbList[n] = new MemBlock(sizeof(SlfHeader) + n*pageSize_*BLOCK_SIZE, pageSize_);
    }
    mbList[n]->isDirty = true;
    return write(mbList[n]->getP(fpos) );

}

template <typename DataType, typename LockType, typename Alloc> bool SkipNode<
DataType, LockType, Alloc>::write(char* pBuf)
{
    if (!isDirty)
    {
        return true;
    }

    if (!isLoaded)
    {
        return true;
    }

    char* p = pBuf;

    if ( !pBuf)
    {
        return false;
    }

    size_t recSize;

    memcpy(p, &height, sizeof(int));
    p += sizeof(int);
    for (int i=0; i<height; i++)
    {
        if (right[i])
        {
            rfpos[i] = right[i]->fpos;

        }
        else
        {
            rfpos[i] = 0;
        }

    }

    memcpy(p, &rfpos[0], sizeof(long)*MAX_LEVEL);
    p += sizeof(long)*MAX_LEVEL;

    object_.reset(new DbObj);
    write_image(element, object_);

    // write the size of the DbObj.
    recSize = object_->getSize();
    memcpy(p, &recSize, sizeof(size_t));
    p += sizeof(size_t);

    char *pd;
    pd = ( char* )object_->getData();
    if (recSize <= maxDataSize_)
    {
        memcpy(p, pd, recSize);
        p += recSize;
    }
    else
    {
        assert(false);
        /*if (1 != fwrite(pd, maxDataSize_, 1, f)) {
         lock_.release_write_lock();
         return false;
         }
         if (1 != fwrite(&overFlowAddress, sizeof(long), 1, f) ) {
         lock_.release_write_lock();
         return false;
         }
         if (0 != fseek(f, overFlowAddress, SEEK_SET)) {
         lock_.release_write_lock();
         return false;
         }
         //update would waste the disk, need to improve here.
         if (1 != fwrite(pd + maxDataSize_, recSize - maxDataSize_, 1, f)) {
         lock_.release_write_lock();
         return false;
         }*/
    }

    isLoaded = true;
    isDirty = false;
    return true;

}

NS_IZENELIB_AM_END
#endif /*SKIPNODE_H_*/

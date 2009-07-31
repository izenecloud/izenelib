#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <types.h>

NS_IZENELIB_UTIL_BEGIN

#define DYNARRAY_DEFAULTBLKSIZE	1024//128

template<class T>
class Const_NullValue
{
public:
    inline operator T()const
    {
        return (T)0;
    }
    inline operator int()const
    {
        return (int)0;
    }
};

template<class ElemT,class NullValue = Const_NullValue<ElemT> >
class DynamicArray
{
public:
    typedef ElemT element_type;
public:
    DynamicArray(void);

    DynamicArray(int32_t blkSize_);

    DynamicArray(size_t initSize,int32_t blkSize_);

    DynamicArray(const DynamicArray& src);

    ~DynamicArray(void);
public:
    class DynamicArrayIterator
    {
    public:
        DynamicArrayIterator(DynamicArray<ElemT,NullValue>* pparent)
                :pParent_(pparent)
                ,curBlk_(-1)
                ,curOffset_(-1)
        {
        }
        DynamicArrayIterator(const DynamicArrayIterator& clone)
                :pParent_(clone.pParent_)
                ,curBlk_(clone.curBlk_)
                ,curOffset_(clone.curOffset_)
        {
        }
        ~DynamicArrayIterator() {}
    public:
        bool next()
        {
            if (curBlk_ == -1)
            {
                curBlk_ = pParent_->blkBase_;
                while (!pParent_->blocks_[curBlk_] && (curBlk_ < pParent_->numBlks_) )
                    curBlk_++;
                if (curBlk_ >= pParent_->numBlks_)
                    return false;
            }
            do
            {
                curOffset_++;
                if (curOffset_ >= pParent_->blkSize_)
                {
                    curBlk_++;
                    while (!pParent_->blocks_[curBlk_] && (curBlk_ < pParent_->numBlks_) )
                        curBlk_++;
                    if (curBlk_>=pParent_->numBlks_)
                        return false;
                    curOffset_ = 0;
                }
            }
            while (pParent_->blocks_[curBlk_][curOffset_] == (ElemT)NullValue());

            return true;
        }
        element_type element()
        {
            return pParent_->blocks_[curBlk_][curOffset_];
        }
        size_t position()
        {
            return curBlk_*pParent_->blkSize_ + curOffset_;
        }
    private:
        DynamicArray<ElemT,NullValue>*pParent_;
        int32_t curBlk_;
        int32_t curOffset_;
    };

    typedef DynamicArrayIterator array_iterator;
public:
    ElemT& operator[](size_t order);

    ElemT getAt(size_t order);

    void insert(size_t order,const ElemT& val);

    array_iterator elements();

    size_t length();

    size_t setMaxlength(size_t newMaxLen);

    void clear();

    void reset();
private:
    int32_t block(size_t order);

    int32_t offset(size_t order);

    void grow(size_t newLen);

    void allocBlock(int32_t blk);
private:
	
    ElemT** blocks_;
    int32_t	blkSize_;
    int32_t	blkBase_;
    int32_t	numBlks_;
    size_t maxLength_;
};
//////////////////////////////////////////////////////////////////////////
//
template<class ElemT,class NullValue>
DynamicArray<ElemT,NullValue>::DynamicArray(void)
        :blkSize_(DYNARRAY_DEFAULTBLKSIZE)
        ,blkBase_(0)
        ,numBlks_(1)
        ,maxLength_(DYNARRAY_DEFAULTBLKSIZE)
{
    blocks_ = new ElemT*[numBlks_];
    blocks_[0] = NULL;
}

template<class ElemT,class NullValue>
DynamicArray<ElemT,NullValue>::DynamicArray(int32_t blksize)
        :blkSize_(blksize)
        ,blkBase_(0)
        ,numBlks_(1)
        ,maxLength_(blksize)
{
    blocks_ = new ElemT*[numBlks_];
    blocks_[0] = NULL;
}
template<class ElemT,class NullValue>
DynamicArray<ElemT,NullValue>::DynamicArray(size_t initSize,int32_t blksize)
        :blkSize_(blksize)
        ,blkBase_(0)
        ,maxLength_(initSize)
{
    numBlks_ = (int32_t)((initSize + blksize - 1)/blksize);
    blocks_ = new ElemT*[numBlks_];
    
    memset(blocks_,0,numBlks_*sizeof(ElemT*));
}

template<class ElemT,class NullValue>
DynamicArray<ElemT,NullValue>::DynamicArray(const DynamicArray& src)
{
    blkSize_ = src.blkSize_;
    blkBase_ = src.blkBase_;
    numBlks_ = src.numBlks_;
    maxLength_ = src.maxLength_;
    blocks_ = new ElemT*[numBlks_];
    memcpy(blocks_,src.blocks_,numBlks_*sizeof(ElemT*));
    for (int32_t i = 0;i<numBlks_;i++)
    {
        allocBlock(i);
        memcpy(blocks_[i],src.blocks_[i],blkSize_*sizeof(ElemT));
    }
}

template<class ElemT,class NullValue>
DynamicArray<ElemT,NullValue>::~DynamicArray(void)
{
    clear();
}

template<class ElemT,class NullValue>
ElemT& DynamicArray<ElemT,NullValue>::operator[](size_t order)
{
    int32_t blk = block(order);
    int32_t off = offset(order);

//    if (order >= maxLength_)
    if(blk >= numBlks_)
        grow(order+1);
    if (blocks_[blk] == NULL)
        allocBlock(blk);
    return blocks_[blk][off];
}

template<class ElemT,class NullValue>
ElemT DynamicArray<ElemT,NullValue>::getAt(size_t order)
{
    if (order >= maxLength_)
        return NULL;
    int32_t blk = block(order);
    int32_t off = offset(order);
    if (blocks_[blk] == NULL)
        return NULL;
    return blocks_[blk][off];
}

template<class ElemT,class NullValue>
typename DynamicArray<ElemT,NullValue>::array_iterator DynamicArray<ElemT,NullValue>::elements()
{
    return typename DynamicArray<ElemT,NullValue>::array_iterator(this);
}

template<class ElemT,class NullValue>
void DynamicArray<ElemT,NullValue>::insert(size_t order,const ElemT& val)
{
    if (order >= maxLength_)
        grow(order+1);
    int32_t blk = block(order);
    int32_t off = offset(order);
    if (blocks_[blk] == NULL)
        allocBlock(blk);
    blocks_[blk][off] = val;
}

template<class ElemT,class NullValue>
void DynamicArray<ElemT,NullValue>::grow(size_t newLen)
{
    int32_t numBlks = (int32_t)((newLen + blkSize_ - 1)/blkSize_);

    ElemT** tmpBlks = new ElemT*[numBlks];
	
    memset(tmpBlks,0,numBlks*sizeof(ElemT*));
    memcpy(tmpBlks,blocks_,numBlks_*sizeof(ElemT*));
    delete[] blocks_;
    blocks_ = tmpBlks;
    numBlks_ = numBlks;
    maxLength_ = newLen;
}

template<class ElemT,class NullValue>
void DynamicArray<ElemT,NullValue>::allocBlock(int32_t blk)
{
    blocks_[blk] = new ElemT[blkSize_];

    for (int32_t i = 0;i<blkSize_;i++)
    {
        blocks_[blk][i] = NullValue();
    }
}

template<class ElemT,class NullValue>
void DynamicArray<ElemT,NullValue>::clear()
{
    if (blocks_)
    {
    
        for (int32_t i = 0;i<numBlks_;i++)
        {
            if (blocks_[i])
                delete[] blocks_[i];
        }
        delete[] blocks_;
        blocks_ = NULL;

        numBlks_ = 0;
    }
    maxLength_ = 0;
}
template<class ElemT,class NullValue>
void DynamicArray<ElemT,NullValue>::reset()
{
    if (blocks_)
    {
        for (int32_t i = 0;i<numBlks_;i++)
        {
            if (blocks_[i])
            {
                memset(blocks_[i],(int)NullValue(),blkSize_*sizeof(ElemT));
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////
//inline functions
template<class ElemT,class NullValue>
inline size_t DynamicArray<ElemT,NullValue>::length()
{
    int32_t length = 0;
    for (int32_t i = 0;i<numBlks_;i++)
    {
        if (blocks_[i - blkBase_])
        {
            for (int32_t j = 0;j<blkSize_;j++)
                if (blocks_[i - blkBase_][j] != (ElemT)NullValue())
                    length++;
        }
    }
    return length;
}
template<class ElemT,class NullValue>
inline size_t DynamicArray<ElemT,NullValue>::setMaxlength(size_t newMaxLen)
{
    if (newMaxLen > maxLength_)
    {
        grow(newMaxLen);
        return newMaxLen;
    }
    else return maxLength_;
}
template<class ElemT,class NullValue>
inline int32_t DynamicArray<ElemT,NullValue>::block(size_t order)
{
    return ((int32_t)(order/blkSize_) - blkBase_);
}
template<class ElemT,class NullValue>
inline int32_t DynamicArray<ElemT,NullValue>::offset(size_t order)
{
    return (int32_t)(order%blkSize_);
}


NS_IZENELIB_UTIL_END

#endif

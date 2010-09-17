/**
* @file        BitVector.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief 
*/
#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/*
*@brief BitVector
* An implementation of bitmap
*/
class BitVector
{
public:
    BitVector():bits_(0), size_(0) {}

    BitVector(const BitVector& other)
        :size_(other.size_)
    {
        blockNum_ = (size_ >> 3) + 1;
        bits_ = new unsigned char[blockNum_];
        clear();
    }

    BitVector(size_t n)
        :size_(n)
    {
        blockNum_ = (size_ >> 3) + 1;
        bits_ = new unsigned char[blockNum_];
        clear();
    }
    ~BitVector()
    {
        if(bits_)
        {
            delete[] bits_;
            bits_ = 0;
        }
    }

public:
    void set(size_t bit) 
    {
        if(bit >= size_)
            grow(bit+4);
        bits_[bit >> 3] |= 1 << (bit & 7);
    }

    void clear(size_t bit) 
    {
        if(bit >= size_)
            grow(bit+4);
        bits_[bit >> 3] &= ~(1 << (bit & 7));
    }

    void clear() { memset(bits_, 0 , blockNum_); }

    void setAll() { memset(bits_, 0xFF, blockNum_);}

    bool test(size_t bit)
    {
        if(bit >= size_)
            return false;
        return bits_[bit >> 3] & (1 << (bit & 7));
    }

    bool any()
    {
        for (size_t i = 0; i < blockNum_; ++i)
            if (bits_[i])
                return true;
        return false;
    }

    void toggle()
    {
        for(size_t i = 0; i < blockNum_; ++i )
            bits_[i] = ~bits_[i];
    }

    BitVector& operator&=(const BitVector& b)
    {
        for(size_t i = 0; i < blockNum_; ++i )
            bits_[i] &= b.bits_[i];
        return *this;
    }

    BitVector& operator|=(const BitVector& b)
    {
        for(size_t i = 0; i < blockNum_; ++i )
            bits_[i] |= b.bits_[i];
        return *this;
    }

    BitVector& operator^=(const BitVector& b)
    {
        for(size_t i = 0; i < blockNum_; ++i )
            bits_[i] ^= b.bits_[i];
        return *this;
    }

    size_t size() { return size_; }

    void read(Directory* pDirectory,const char* name)
    {
        IndexInput* pInput = pDirectory->openInput(name);
        size_= (size_t)pInput->readInt();
        blockNum_ = (size_ >> 3) + 1;
        bits_ = new unsigned char[blockNum_];
        clear();
        pInput->read((char*)bits_,blockNum_*sizeof(unsigned char));
        delete pInput;
    }

    void write(Directory* pDirectory,const char* name)
    {
        IndexOutput* pOutput = pDirectory->createOutput(name);
        pOutput->writeInt((int32_t)size_);
        pOutput->write((const char*)bits_,blockNum_*sizeof(unsigned char));
        delete pOutput;
    }

    bool hasSmallThan(size_t bit) const
    {
        if(bit >= size_)
        {
            for (size_t i = 0; i < blockNum_; ++i)
                if (bits_[i])
                    return true;
            return false;
        }
        size_t nTestBlk = bit >> 3;
        for (size_t i = 0; i <= nTestBlk; ++i)
            if (bits_[i])
                return true;
        return false;
    }

    bool hasBetween(size_t start, size_t end) const
    {
        size_t nStartBlk = start >> 3;
        size_t nEndBlk = end >> 3;
        for (size_t i = nStartBlk; i <= nEndBlk; ++i)
            if (bits_[i])
                return true;

        return false;
    }

    size_t getMaxSet()
    {
        size_t i;
        for(i = size_-1; i >=0; --i)
        {
            if(test(i))
                return i;
        }
        return i;
    }

private:
    void grow(size_t length)
    {
        size_ = length;
        size_t newBlockNum = (size_ >> 3) + 1;
        unsigned char* newBits_ = new unsigned char[newBlockNum];
        memset(newBits_,0,newBlockNum*sizeof(unsigned char));
        memcpy(newBits_,bits_,blockNum_*sizeof(unsigned char));
        delete bits_;
        bits_ = newBits_;
        blockNum_ = newBlockNum;
    }

private:
    unsigned char* bits_;
    size_t size_;
    size_t blockNum_;
};


}

NS_IZENELIB_IR_END

#endif

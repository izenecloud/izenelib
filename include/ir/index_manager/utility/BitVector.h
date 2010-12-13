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
    BitVector():bits_(0), size_(0), blockNum_(0) {}

    BitVector(const BitVector& other)
        :size_(other.size_)
    {
        blockNum_ = getBlockNum(size_);
        bits_ = new unsigned char[blockNum_];
        clear();
    }

    BitVector(size_t n)
        :size_(n)
    {
        blockNum_ = getBlockNum(size_);
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
        blockNum_ = getBlockNum(size_);
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

    /**
     * Check whether there is any bit in the range [0, @p bit] is set to 1.
     * @param bit it would check the range of bits [0, bit]
     * @return true for the bit is found, false for no bit in the range is set to 1
     */
    bool hasSmallThan(size_t bit) const
    {
        size_t endBlk = 0;
        if(bit < size_)
        {
            endBlk = bit >> 3;
            unsigned char headMask = (1 << ((bit & 7) + 1)) - 1;
            if(bits_[endBlk] & headMask)
                return true;
        }
        else
            endBlk = blockNum_;

        for (size_t i = 0; i < endBlk; ++i)
            if (bits_[i])
                return true;
        return false;
    }

    /**
     * Check whether there is any bit in the range [@p start, @p end] is set to 1.
     * @param start start position
     * @param end end position, it would check the range of bits [start, end]
     * @return true for the bit is found, false for no bit in the range is set to 1
     * @pre @p start <= @p end
     */
    bool hasBetween(size_t start, size_t end) const
    {
        assert(start <= end);

        if(start >= size_)
            return false;
        if(end >= size_)
            end = size_ - 1;

        unsigned char startMask = ~((1 << (start & 7)) - 1);
        size_t startBlk = start >> 3;
        if(bits_[startBlk] & startMask)
            return true;

        unsigned char endMask = (1 << ((end & 7) + 1)) - 1;
        size_t endBlk = end >> 3;
        if(bits_[endBlk] & endMask)
            return true;

        for (size_t i = startBlk+1; i < endBlk; ++i)
            if (bits_[i])
                return true;

        return false;
    }

private:
    void grow(size_t length)
    {
        size_ = length;
        size_t newBlockNum = getBlockNum(size_);
        unsigned char* newBits_ = new unsigned char[newBlockNum];
        memset(newBits_,0,newBlockNum*sizeof(unsigned char));
        memcpy(newBits_,bits_,blockNum_*sizeof(unsigned char));
        delete[] bits_;
        bits_ = newBits_;
        blockNum_ = newBlockNum;
    }

    /**
     * Get the number of bytes to contain @p bitNum bits.
     * @param bitNum the number of bits
     * @return the number of bytes
     */
    size_t getBlockNum(size_t bitNum) const
    {
        if(bitNum == 0)
            return 0;

        return ((bitNum - 1) >> 3) + 1;
    }

private:
    unsigned char* bits_; ///< bit vector, each continuous 8 bits are represented as one byte
    size_t size_; ///< size in bits
    size_t blockNum_; ///< size in bytes
};


}

NS_IZENELIB_IR_END

#endif

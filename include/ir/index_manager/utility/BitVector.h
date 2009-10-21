#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class BitVector
{
public:
    BitVector():bits_(0), size_(0) {}
	
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
            delete bits_;
    }

public:
    void set(size_t bit) 
    {
        if(bit >= size_)
            grow(bit+1);
        bits_[bit >> 3] |= 1 << (bit & 7);
    }

    void clear(size_t bit) 
    {
        if(bit >= size_)
            grow(bit+1);
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

private:
    void grow(size_t length)
    {
        unsigned char* newBits_ = new unsigned char[length];
        memset(newBits_,0,length);
        memcpy(newBits_,bits_,size_);
        size_ = length;
        blockNum_ = (size_ >> 3) + 1;
        delete bits_;
        bits_ = newBits_;
    }

private:
    unsigned char* bits_;
    size_t size_;
    size_t blockNum_;
};


}

NS_IZENELIB_IR_END

#endif

#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class BitVector
{
public:
    BitVector(size_t n)
        :size_(n)
    {
        bits_ = new unsigned char[(size_ >> 3) + 1];
        clear();
    }
    ~BitVector()
    {
        delete bits_;
    }

public:
    void set(size_t bit) 
    {
        if(bit >= size_)
            return;
        bits_[bit >> 3] |= 1 << (bit & 7);
    }

    void clear(size_t bit) 
    {
        if(bit >= size_)
            return;
        bits_[bit >> 3] &= ~(1 << (bit & 7));
    }

    void clear() { memset(bits_, 0 , size_); }

    void setAll() { memset(bits_, 0xFF, size_);}

    bool test(size_t bit)
    {
        if(bit >= size_)
            return false;
        return (bool)bits_[bit >> 3] & (1 << (bit & 7));
    }

    void toggle()
    {
        for(size_t i = 0; i < size_; ++i )
            bits_[i] = ~bits_[i];
    }

    BitVector& operator&=(const BitVector& b)
    {
        for(size_t i = 0; i < size_; ++i )
            bits_[i] &= b.bits_[i];
        return *this;
    }

    BitVector& operator|=(const BitVector& b)
    {
        for(size_t i = 0; i < size_; ++i )
            bits_[i] |= b.bits_[i];
        return *this;
    }

    BitVector& operator^=(const BitVector& b)
    {
        for(size_t i = 0; i < size_; ++i )
            bits_[i] ^= b.bits_[i];
        return *this;
    }

    size_t size() { return size_; }
private:
    unsigned char* bits_;
    size_t size_;

};


}

NS_IZENELIB_IR_END

#endif

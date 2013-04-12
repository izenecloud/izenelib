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

#include <am/bitmap/Ewah.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
using namespace izenelib::am;
/*
*@brief BitVector
* An implementation of bitmap
*/
class BitVector
{
public:
    BitVector()
        : bits_(0), size_(0), maxBytesNum_(MAX_BYTES_NUM_INIT)
    {
        bits_ = new unsigned char[maxBytesNum_];
        memset(bits_, 0 , maxBytesNum_);
    }

    BitVector(const BitVector& other)
        : bits_(0), size_(other.size_), maxBytesNum_(other.maxBytesNum_)
    {
        bits_ = new unsigned char[maxBytesNum_];
        for(size_t i = 0; i < maxBytesNum_; ++i )
            bits_[i] = other.bits_[i];
    }

    BitVector(size_t n)
        : bits_(0), size_(n), maxBytesNum_(getMaxBytesNum(size_))
    {
        bits_ = new unsigned char[maxBytesNum_];
        memset(bits_, 0 , maxBytesNum_);
    }

    ~BitVector()
    {
        delete[] bits_;
    }

public:
    void set(size_t bit)
    {
        const size_t byte = bit >> 3;
        if(byte >= maxBytesNum_)
            grow(bit + 1);
        if(bit >= size_)
            size_ = bit + 1;

        bits_[byte] |= 1 << (bit & 7);
    }

    void clear(size_t bit)
    {
        const size_t byte = bit >> 3;
        if(byte >= maxBytesNum_)
            grow(bit + 1);
        if(bit >= size_)
            size_ = bit + 1;

        bits_[byte] &= ~(1 << (bit & 7));
    }

    void clear()
    {
        memset(bits_, 0 , getBytesNum(size_));
    }

    void setAll()
    {
        if(size_ == 0)
            return;

        const size_t fullByteNum = getBytesNum(size_) - 1;
        memset(bits_, 0xFF, fullByteNum);

        const size_t endBit = size_ - 1;
        unsigned char endMask = (1 << ((endBit & 7) + 1)) - 1;
        bits_[endBit >> 3] |= endMask;
    }

    bool test(size_t bit) const
    {
        if(bit >= size_)
            return false;

        return bits_[bit >> 3] & (1 << (bit & 7));
    }

    void compressed(EWAHBoolArray<uint32_t>& compressedBitMap) const
    {
        const size_t byteNum = getBytesNum(size_);
        const size_t wordByteNum = sizeof(uint32_t);

        const unsigned int wordNum = byteNum / wordByteNum;
        unsigned int* pWord = reinterpret_cast<unsigned int*>(bits_);
        unsigned int* pWordEnd = pWord + wordNum;
        for (; pWord < pWordEnd; ++pWord)
        {
            compressedBitMap.add(*pWord);
        }

        const unsigned int leftByteNum = byteNum % wordByteNum;
        if (leftByteNum > 0)
        {
            uint32_t lastWord = 0;
            unsigned char* pByte = reinterpret_cast<unsigned char*>(pWordEnd);
            for (unsigned int i = 0; i < leftByteNum; ++i)
            {
                lastWord |= pByte[i] << (i << 3);
            }
            compressedBitMap.add(lastWord);
        }
    }

    void importFromEWAH(const EWAHBoolArray<uint32_t>& ewahBitmap)
    {
        uint pointer(0);
        uint currentoffset(0);
        const vector<uint32_t>& buffer = ewahBitmap.getBuffer();
        while (pointer <buffer.size())
        {
            ConstRunningLengthWord<uint32_t> rlw(buffer[pointer]);
            if (rlw.getRunningBit())
            {
                for (uint x = 0; x < static_cast<uint>(rlw.getRunningLength() * 32); ++x)
                {
                    set(currentoffset + x);
                }
            }
            currentoffset+=rlw.getRunningLength()*32;
            ++pointer;
            for (uint k = 0; k<rlw.getNumberOfLiteralWords();++k)
            {
                const uint32_t currentword = buffer[pointer];
                for (uint k = 0; k < 32; ++k)
                {
                    if ( (currentword & (static_cast<uint32_t>(1) << k)) != 0)
                        set(currentoffset + k);
                }
                currentoffset+=32;
                ++pointer;
            }
        }
    }

    bool any() const
    {
        const size_t byteNum = getBytesNum(size_);
        for(size_t i = 0; i < byteNum; ++i)
            if(bits_[i])
                return true;

        return false;
    }

    std::size_t count() const
    {
        std::size_t count = 0;
        for(std::size_t i=0;i<size();i++)
        {
            if(test(i)) ++count;
        }
        return count;
    }

    void toggle()
    {
        const size_t byteNum = getBytesNum(size_);
        for(size_t i = 0; i < byteNum; ++i )
            bits_[i] = ~bits_[i];
    }

    friend std::ostream& operator<<(std::ostream& output, const BitVector& bv) {
        output<<"["<<bv.size()<<"] ";
        for(std::size_t i=0;i<bv.size();i++)
        {
            output<<(int)bv.test(i);
        }
        return output;
    }

    BitVector& operator&=(const BitVector& b)
    {
        if(size()<b.size())
        {
            grow(b.size());
            size_ = b.size();
        }
        const size_t byteNum = getBytesNum(size_);
        for(size_t i = 0; i < byteNum; ++i )
            bits_[i] &= b.bits_[i];
        return *this;
    }

    bool operator==(const BitVector& b) const
    {
        if(size()!=b.size())
        {
            return false;
        }
        for(std::size_t i=0;i<size();i++)
        {
            if(test(i)!=b.test(i)) return false;
        }
        return true;
    }

    bool equal_ignore_size(const BitVector& b) const
    {
        std::size_t c_size = std::min(size(), b.size());
        for(std::size_t i=0;i<c_size;i++)
        {
            if(test(i)!=b.test(i)) return false;
        }
        if(size()>c_size)
        {
            for(std::size_t i=c_size;i<size();i++)
            {
                if(test(i)) return false;//should be 0
            }
        }
        if(b.size()>c_size)
        {
            for(std::size_t i=c_size;i<b.size();i++)
            {
                if(b.test(i)) return false;//should be 0
            }
        }
        return true;
    }

    BitVector& operator|=(const BitVector& b)
    {
        if(size()<b.size())
        {
            grow(b.size());
            size_ = b.size();
        }
        const size_t byteNum = getBytesNum(size_);
        for(size_t i = 0; i < byteNum; ++i )
            bits_[i] |= b.bits_[i];
        return *this;
    }

    BitVector& operator^=(const BitVector& b)
    {
        if(size()<b.size())
        {
            grow(b.size());
            size_ = b.size();
        }
        const size_t byteNum = getBytesNum(size_);
        for(size_t i = 0; i < byteNum; ++i )
            bits_[i] ^= b.bits_[i];
        return *this;
    }

    void logicalNot(BitVector& b)
    {
        const size_t byteNum = getBytesNum(size_);
        for(size_t i = 0; i < byteNum; ++i )
            b.bits_[i] = ~bits_[i];
     }

    void logicalNotAnd(const BitVector& b)
    {
        const size_t byteNum = getBytesNum(size_);
        for( size_t i = 0; i < byteNum; ++i )
        {
            bits_[i] &= ~b.bits_[i];
        }
    }

    void printout(ostream &o = cout) {
          for(uint k = 1; k < size_; ++k)
              o << test(k) << " ";
          o << endl;
      }

    size_t size() const{ return size_; }

    void read(Directory* pDirectory,const char* name)
    {
        IndexInput* pInput = pDirectory->openInput(name);
        size_= (size_t)pInput->readInt();
        const size_t newBytesNum = getMaxBytesNum(size_);
        if(newBytesNum > maxBytesNum_)
        {
            unsigned char* newBits = new unsigned char[newBytesNum];
            delete[] bits_;
            bits_ = newBits;
            maxBytesNum_ = newBytesNum;
        }
        memset(bits_, 0 , maxBytesNum_);
        pInput->read((char*)bits_, getBytesNum(size_) * sizeof(unsigned char));
        delete pInput;
    }

    void write(Directory* pDirectory,const char* name)
    {
        IndexOutput* pOutput = pDirectory->createOutput(name);
        pOutput->writeInt((int32_t)size_);
        pOutput->write((const char*)bits_, getBytesNum(size_) * sizeof(unsigned char));
        delete pOutput;
    }

    /**
     * Check whether there is any bit in the range [0, @p bit] is set to 1.
     * @param bit it would check the range of bits [0, bit]
     * @return true for the bit is found, false for no bit in the range is set to 1
     */
    bool hasSmallThan(size_t bit) const
    {
        size_t endByte = 0;
        if(bit < size_)
        {
            endByte = bit >> 3;
            unsigned char endMask = (1 << ((bit & 7) + 1)) - 1;
            if(bits_[endByte] & endMask)
                return true;
        }
        else
            endByte = getBytesNum(size_);

        for (size_t i = 0; i < endByte; ++i)
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
        unsigned char endMask = (1 << ((end & 7) + 1)) - 1;
        size_t startByte = start >> 3;
        size_t endByte = end >> 3;

        if(startByte == endByte)
        {
            if(bits_[startByte] & (startMask & endMask))
                return true;
        }
        else
        {
            if((bits_[startByte] & startMask) || (bits_[endByte] & endMask))
                return true;

            for (size_t i = startByte+1; i < endByte; ++i)
                if (bits_[i])
                    return true;
        }

        return false;
    }

private:
    /**
     * Get the number of bytes to contain @p bitNum bits.
     * @param bitNum the number of bits
     * @return the number of bytes
     */
    size_t getBytesNum(size_t bitNum) const
    {
        if(bitNum == 0)
            return 0;

        return ((bitNum - 1) >> 3) + 1;
    }

    /**
     * Get the max number of bytes to contain @p bitNum bits.
     * The initial value is @c MAX_BYTES_NUM_INIT,
     * and it's doubled until @p bitNum is reached.
     * @param bitNum the number of bits
     * @return the max number of bytes
     */
    size_t getMaxBytesNum(size_t bitNum) const
    {
        const size_t bytesNum = getBytesNum(bitNum);
        size_t result = MAX_BYTES_NUM_INIT;

        while(result < bytesNum)
            result <<= 1;

        return result;
    }

    /**
     * Grow @p bits_ to contain @p bitNum bits.
     * @param bitNum the number of bits
     */
    void grow(size_t bitNum)
    {
        const size_t newBytesNum = getBytesNum(bitNum);
       // assert(newBytesNum > maxBytesNum_);
        size_t newMax = maxBytesNum_;
        while(newMax < newBytesNum)
            newMax <<= 1;

        unsigned char* newBits = new unsigned char[newMax];
        memset(newBits, 0, newMax);
        memcpy(newBits, bits_, getBytesNum(size_));
        delete[] bits_;
        bits_ = newBits;
        maxBytesNum_ = newMax;
    }

private:
    unsigned char* bits_; ///< bit vector, each continuous 8 bits are represented as one byte
    size_t size_; ///< the number of bits
    size_t maxBytesNum_; ///< the number of bytes allocated

    enum
    {
        /**
         * the initial number of bytes to allocate.
         * This value is defined to allocate 10M bits at initial time,
         * and the number of bytes would be doubled if necessary.
         *
         * The number "10M" is selected to suffice the collection size of 10M documents.
         * In that case, memory re-allocation would not happen.
         * If "set()" and "test()" are called by multi-threads,
         * the problem of concurrent write and read to @p bits_ could be avoided.
         */
        MAX_BYTES_NUM_INIT = (10 * 1024 * 1024) / 8,
    };
};


}

NS_IZENELIB_IR_END

#endif

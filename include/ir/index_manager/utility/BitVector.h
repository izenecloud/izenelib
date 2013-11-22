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

#include <am/bitmap/ewah.h>
#include <boost/detail/endian.hpp>
#include <emmintrin.h>

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
        ensureInit();
    }

    ~BitVector()
    {
        if (bits_)
            delete[] bits_;
    }

public:
    typedef __m128i block_t;
    typedef void(*fast_opfunc)(block_t&, const block_t&);
    typedef void(*byte_opfunc)(unsigned char&, const unsigned char&);
    typedef bool(*fast_checkfunc)(const uint64_t&, const uint64_t&);
    typedef bool(*byte_checkfunc)(const unsigned char&, const unsigned char&);

    void set(size_t bit)
    {
        ensureInit();
        const size_t byte = bit >> 3;
        if(byte >= maxBytesNum_)
            grow(bit + 1);
        if(bit >= size_)
            size_ = bit + 1;

        bits_[byte] |= 1 << (bit & 7);
    }

    void clear(size_t bit)
    {
        ensureInit();
        const size_t byte = bit >> 3;
        if(byte >= maxBytesNum_)
            grow(bit + 1);
        if(bit >= size_)
            size_ = bit + 1;

        bits_[byte] &= ~(1 << (bit & 7));
    }

    void clear()
    {
        ensureInit();
        memset(bits_, 0 , getBytesNum(size_));
    }

    void setAll()
    {
        if(size_ == 0)
            return;

        ensureInit();
        const size_t byteNum = getBytesNum(size_);
        memset(bits_, 0xFF, byteNum);

        clearDirtyBits();
    }

    bool test(size_t bit) const
    {
        if(bit >= size_)
            return false;

        return bits_[bit >> 3] & (1 << (bit & 7));
    }

    template <typename word_t>
    void compressed(EWAHBoolArray<word_t>& compressedBitMap) const
    {
        const size_t byteNum = getBytesNum(size_);
        const size_t wordByteNum = sizeof(word_t);

        const size_t wordNum = byteNum / wordByteNum;
        word_t* pWord = reinterpret_cast<word_t*>(bits_);
        for (std::size_t i = 0; i < wordNum; ++i)
        {
            word_t word = getWord(pWord++);
            compressedBitMap.add(word);
        }

        const size_t leftByteNum = byteNum % wordByteNum;
        if (leftByteNum > 0)
        {
            unsigned char* pBytes = reinterpret_cast<unsigned char*>(pWord);
            word_t word = getWord<word_t>(pBytes, leftByteNum);
            compressedBitMap.add(word);
        }
    }

    template <typename word_t>
    void importFromEWAH(const EWAHBoolArray<word_t>& ewahBitmap)
    {
        ensureInit();
        uint pointer(0);
        uint currentoffset(0);
        const uint wordBitNum = sizeof(word_t) << 3;
        const vector<word_t>& buffer = ewahBitmap.getBuffer();

        while (pointer < buffer.size())
        {
            ConstRunningLengthWord<word_t> rlw(buffer[pointer]);
            const uint bitNum = rlw.getRunningLength() * wordBitNum;

            if (rlw.getRunningBit())
            {
                for (uint bi = 0; bi < bitNum; ++bi)
                {
                    set(currentoffset + bi);
                }
            }

            currentoffset += bitNum;
            ++pointer;

            const uint literalWordNum = rlw.getNumberOfLiteralWords();
            for (uint wi = 0; wi < literalWordNum; ++wi)
            {
                const word_t currentWord = buffer[pointer];
                word_t mask = 1;
                for (uint bi = 0; bi < wordBitNum; ++bi)
                {
                    if (currentWord & mask)
                    {
                        set(currentoffset);
                    }
                    mask <<= 1;
                    ++currentoffset;
                }
                ++pointer;
            }
        }
    }

    static void fastand(block_t& l, const block_t& r)
    {
        l = _mm_and_si128(l, r);
    }
    static void byteand(unsigned char& l, const unsigned char& r)
    {
        l &= r;
    }

    static void fastor(block_t& l, const block_t& r)
    {
        l = _mm_or_si128(l, r);
    }
    static void byteor(unsigned char& l, const unsigned char& r)
    {
        l |= r;
    }

    // ^
    static void fastxor(block_t& l, const block_t& r)
    {
        l = _mm_xor_si128(l, r);
    }
    static void bytexor(unsigned char& l, const unsigned char& r)
    {
        l ^= r;
    }

    // L & (~R)
    static void fastnotand(block_t& l, const block_t& r)
    {
        l = _mm_andnot_si128(r, l);
    }
    static void bytenotand(unsigned char& l, const unsigned char& r)
    {
        l &= (~r);
    }

    static void fastnot(block_t& l, const block_t& r)
    {
        static const __m128i allone = _mm_set1_epi8(0xFF);
        l = _mm_andnot_si128(r, allone);
    }
    static void bytenot(unsigned char& l, const unsigned char& r)
    {
        l = ~r;
    }

    static bool fastcmpeq_sse(const block_t& l, const block_t& r)
    {
        // r0 := (a0 == b0) ? 0xffffffff : 0x0
        // r1 := (a1 == b1) ? 0xffffffff : 0x0
        // r2 := (a2 == b2) ? 0xffffffff : 0x0
        // r3 := (a3 == b3) ? 0xffffffff : 0x0
        __m128i ret = _mm_cmpeq_epi32(l, r);
        char tmp[16]; 
        _mm_storeu_si128((__m128i*)tmp, ret);
        for(size_t i = 0; i < 16; ++i)
        {
            if (tmp[i] != 0xFF)
                return false;
        }
        return true;
    }

    static bool bytecmpeq(const unsigned char& l, const unsigned char& r)
    {
        return l == r;
    }

    static bool fastcmpeq(const uint64_t& l, const uint64_t& r)
    {
        return l == r;
    }

    static bool fastcmpnteq(const uint64_t& l, const uint64_t& r)
    {
        return l != r;
    }
    static bool bytecmpnteq(const unsigned char& l, const unsigned char& r)
    {
        return l != r;
    }

    static bool fast_iszero(const uint64_t& l, const uint64_t& r)
    {
        return l == 0;
    }
    static bool byte_iszero(const unsigned char& l, const unsigned char& r)
    {
        return l == 0;
    }

    void fast_bitwise_op(fast_opfunc op, byte_opfunc byteop)
    {
        return fast_bitwise_op(op, byteop, *this);
    }

    void fast_bitwise_op(fast_opfunc op, byte_opfunc byteop, const BitVector& other)
    {
        const size_t min_size = std::min(size_, other.size_);
        const size_t byteNum = getBytesNum(min_size);
        const size_t blockByteNum = sizeof(block_t);

        const size_t blockNum = byteNum / blockByteNum;
        block_t* pBlock = reinterpret_cast<block_t*>(bits_);
        const block_t* potherBlock = reinterpret_cast<const block_t*>(other.bits_);
        for (std::size_t i = 0; i < blockNum; ++i)
        {
            __m128i tmp = _mm_loadu_si128(pBlock);
            op(tmp, _mm_loadu_si128(potherBlock));
            _mm_storeu_si128(pBlock, tmp);
            ++pBlock;
            ++potherBlock;
        }

        const size_t leftByteNum = byteNum % blockByteNum;
        if (leftByteNum > 0)
        {
            unsigned char* pBytes = reinterpret_cast<unsigned char*>(pBlock);
            const unsigned char* potherBytes = reinterpret_cast<const unsigned char*>(potherBlock);
            for (std::size_t i = 0; i < leftByteNum; ++i)
            {
                byteop(*pBytes, *potherBytes);
                ++pBytes;
                ++potherBytes;
            }
        }
    }

    bool fast_check_op(fast_checkfunc ck, byte_checkfunc byteck, const BitVector& other) const
    {
        typedef uint64_t block_t;
        const size_t min_size = std::min(size_, other.size_);
        const size_t byteNum = getBytesNum(min_size);
        const size_t blockByteNum = sizeof(block_t);

        const size_t blockNum = byteNum / blockByteNum;
        const block_t* pBlock = reinterpret_cast<const block_t*>(bits_);
        const block_t* potherBlock = reinterpret_cast<const block_t*>(other.bits_);
        for (std::size_t i = 0; i < blockNum; ++i)
        {
            if( !ck (*pBlock, *potherBlock) )
                return false;
            ++pBlock;
            ++potherBlock;
        }

        const size_t leftByteNum = byteNum % blockByteNum;
        if (leftByteNum > 0)
        {
            const unsigned char* pBytes = reinterpret_cast<const unsigned char*>(pBlock);
            const unsigned char* potherBytes = reinterpret_cast<const unsigned char*>(potherBlock);
            for (std::size_t i = 0; i < leftByteNum; ++i)
            {
                if( !byteck(*pBytes, *potherBytes) )
                    return false;
                ++pBytes;
                ++potherBytes;
            }
        }
        return true;
    }

    bool any() const
    {
        return !fast_check_op(&fast_iszero, &byte_iszero, *this);
    }

    std::size_t count() const
    {
        std::size_t count = 0;
        typedef unsigned long long block_t;

        const size_t byteNum = getBytesNum(size_);
        const size_t blockByteNum = sizeof(block_t);

        const size_t blockNum = byteNum / blockByteNum;
        block_t* pBlock = reinterpret_cast<block_t*>(bits_);
        for (std::size_t i = 0; i < blockNum; ++i)
        {
            count += __builtin_popcountll(*pBlock);
            ++pBlock;
        }

        for(std::size_t i = blockNum*blockByteNum*8; i < size(); ++i)
        {
            if ( test(i) ) ++count;
        }
        return count;
    }

    void toggle()
    {
        ensureInit();
        fast_bitwise_op(&fastnot, &bytenot);
        clearDirtyBits();
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
        ensureInit();
        if(size()<b.size())
        {
            grow(b.size());
            size_ = b.size();
        }
        fast_bitwise_op(&fastand, &byteand, b);

        const size_t byteNum = getBytesNum(b.size_);
        const size_t my_byteNum = getBytesNum(size_);
        memset(bits_ + byteNum, 0, my_byteNum - byteNum);
        return *this;
    }

    BitVector& operator=(const BitVector& b)
    {
        if (this != &b )
        {
            const size_t newBytesNum = getMaxBytesNum(b.size_);
            size_ = b.size_;
            if(newBytesNum > maxBytesNum_)
            {
                unsigned char* newBits = new unsigned char[newBytesNum];
                std::copy(b.bits_, b.bits_ + getBytesNum(b.size_), newBits);
                if (bits_)
                    delete[] bits_;
                bits_ = newBits;
                maxBytesNum_ = newBytesNum;
            }
            else
            {
                ensureInit();
                std::copy(b.bits_, b.bits_ + getBytesNum(b.size_), bits_);
            }
        }
        return *this;
    }

    bool operator==(const BitVector& b) const
    {
        if(size() != b.size())
        {
            return false;
        }

        return fast_check_op(&fastcmpeq, &bytecmpeq, b);
    }

    bool equal_ignore_size(const BitVector& b) const
    {
        std::size_t c_size = std::min(size(), b.size());
        if (!fast_check_op(&fastcmpeq, &bytecmpeq, b))
            return false;

        for(size_t i = getBytesNum(c_size); i < getBytesNum(size()); ++i)
        {
            if (bits_[i] != 0)
                return false;
        }
        for(size_t i = getBytesNum(c_size); i < getBytesNum(b.size()); ++i)
        {
            if (b.bits_[i] != 0)
                return false;
        }
        return true;
    }

    BitVector& operator|=(const BitVector& b)
    {
        ensureInit();
        if(size()<b.size())
        {
            grow(b.size());
            size_ = b.size();
        }
        fast_bitwise_op(&fastor, &byteor, b);
        return *this;
    }

    BitVector& operator^=(const BitVector& b)
    {
        ensureInit();
        if(size()<b.size())
        {
            grow(b.size());
            size_ = b.size();
        }

        fast_bitwise_op(&fastxor, &bytexor, b);
        return *this;
    }

    //void logicalNot(const BitVector& b)
    //{
    //    const size_t byteNum = getBytesNum(size_);
    //    for(size_t i = 0; i < byteNum; ++i )
    //        bits_[i] = ~b.bits_[i];
    //}

    void logicalNotAnd(const BitVector& b)
    {
        ensureInit();
        //typedef uint64_t word_t;
        //std::size_t min_size = std::min(size_, b.size_);

        //const size_t byteNum = getBytesNum(min_size);
        //const size_t wordByteNum = sizeof(word_t);

        //const size_t wordNum = byteNum / wordByteNum;
        //word_t* pWord1 = reinterpret_cast<word_t*>(bits_);
        //word_t* pWord2 = reinterpret_cast<word_t*>(b.bits_);
        //for (std::size_t i = 0; i < wordNum; ++i)
        //{
        //    *pWord1++ &= ~*pWord2++;
        //}

        //const size_t leftByteNum = byteNum % wordByteNum;
        //if (leftByteNum > 0)
        //{
        //    unsigned char* pBytes1 = reinterpret_cast<unsigned char*>(pWord1);
        //    unsigned char* pBytes2 = reinterpret_cast<unsigned char*>(pWord2);
        //    for (std::size_t i = 0; i < leftByteNum; ++i)
        //    {
        //        *pBytes1++ &= ~*pBytes2++;
        //    }
        //}
        fast_bitwise_op(&fastnotand, &bytenotand, b);
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
            memset(newBits, 0 , newBytesNum);
            if (bits_)
                delete[] bits_;
            bits_ = newBits;
            maxBytesNum_ = newBytesNum;
        }
        else
        {
            ensureInit();
        }
        pInput->read((char*)bits_, getBytesNum(size_) * sizeof(unsigned char));
        delete pInput;
    }

    void write(Directory* pDirectory,const char* name) const
    {
        IndexOutput* pOutput = pDirectory->createOutput(name);
        pOutput->writeInt((int32_t)size_);
        if (bits_)
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

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version)  const
    {
        ar & size_;
        if (bits_)
            ar.save_binary(bits_, getBytesNum(size_)*sizeof(unsigned char));
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & size_;
        const size_t newBytesNum = getMaxBytesNum(size_);
        if(newBytesNum > maxBytesNum_)
        {
            unsigned char* newBits = new unsigned char[newBytesNum];
            memset(newBits, 0 , newBytesNum);
            if (bits_)
                delete[] bits_;
            bits_ = newBits;
            maxBytesNum_ = newBytesNum;
        }
        else
        {
            ensureInit();
        }
        ar.load_binary(bits_, getBytesNum(size_)*sizeof(unsigned char));
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:

    void ensureInit()
    {
        if (bits_ == NULL)
        {
            bits_ = new unsigned char[maxBytesNum_];
            memset(bits_, 0 , maxBytesNum_);
        }
    }
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
     * Clear the dirty bits which positions are not less than @c size_.
     * For example, if the @c bits_ content is "11111111", and size_ is 4,
     * after calling this function, the last four dirty bits would be cleared,
     * that is, the @c bits content would be "11110000".
     */
    void clearDirtyBits()
    {
        // no dirty bits when size_ is a multiple of 8
        if ((size_ & 7) == 0)
            return;

        ensureInit();
        const size_t endBit = size_ - 1;
        const unsigned char endMask = (1 << ((endBit & 7) + 1)) - 1;
        bits_[endBit >> 3] &= endMask;
    }

    /**
     * Get the word value from the bytes.
     * @param pWord pointer to the bytes, in little endian order.
     * @return the word value
     * @note this function runs faster on little endian machine than on big
     *       endian machine, because the word value could be read out directly
     *       on little endian machine.
     */
    template <typename word_t>
    word_t getWord(word_t* pWord) const
    {
#if defined(BOOST_LITTLE_ENDIAN)
        return *pWord;

#elif defined(BOOST_BIG_ENDIAN)
        unsigned char* pBytes = reinterpret_cast<unsigned char*>(pWord);
        return getWord<word_t>(pBytes, sizeof(word_t));

#else
#error "BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN not defined"
#endif
    }

    /**
     * Get the word value from the bytes.
     * @param pBytes pointer to the bytes, in little endian order.
     * @param byteNum the number of bytes,
     *                it should not be greater than sizeof(word_t).
     * @return the word value
     */
    template <typename word_t>
    word_t getWord(unsigned char* pBytes, std::size_t byteNum) const
    {
        word_t word = 0;

        for (std::size_t i = 0; i < byteNum; ++i)
        {
            word |= static_cast<word_t>(*pBytes++) << (i << 3);
        }

        return word;
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

        if (newMax == maxBytesNum_)
        {
            ensureInit();
            return;
        }

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

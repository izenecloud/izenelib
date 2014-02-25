#ifndef IZENELIB_IR_BITSET_H
#define IZENELIB_IR_BITSET_H

#include <types.h>
#include <ir/index_manager/store/Directory.h>
#include <am/bitmap/ewah.h>
#include <boost/detail/endian.hpp>
#include <boost/shared_array.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

using namespace izenelib::am;

/*
*@brief Bitset
* An implementation of bitmap
*/
class Bitset
{
public:
    Bitset();
    Bitset(size_t size);
    Bitset(size_t size, size_t capacity, const boost::shared_array<uint64_t>& bits);
    Bitset(const Bitset& other, bool dup);
    virtual ~Bitset();

    const Bitset& dup();

    bool test(size_t pos) const;
    bool any() const;
    bool none() const;
    size_t count() const;
    bool any(size_t start, size_t end) const;
    size_t count(size_t start, size_t end) const;

    void set(size_t pos);
    void set();
    void reset(size_t pos);
    void reset();
    void flip(size_t pos);
    void flip();

    size_t find_first() const;
    size_t find_next(size_t pos) const;
    size_t find_last() const;
    size_t find_prev(size_t pos) const;
    size_t select(size_t ind) const;

    bool operator==(const Bitset& b) const;
    bool equal_ignore_size(const Bitset& b) const;
    Bitset& operator&=(const Bitset& b);
    Bitset& operator|=(const Bitset& b);
    Bitset& operator^=(const Bitset& b);
    Bitset& operator-=(const Bitset& b);

    size_t size() const;
    void swap(Bitset& b);

    void read(Directory* pDirectory, const char* name);
    void write(Directory* pDirectory, const char* name) const;

    template <typename word_t>
    void compress(EWAHBoolArray<word_t>& compressedBitMap) const
    {
        const size_t byteNum = block_num(size_) * sizeof(uint64_t);
        const size_t wordByteNum = sizeof(word_t);

        const size_t wordNum = byteNum / wordByteNum;
        const word_t* pWord = reinterpret_cast<const word_t*>(bits_.get());
        const word_t* pWordEnd = pWord + wordNum;
        while (pWord != pWordEnd)
        {
            compressedBitMap.add(*pWord++);
        }
    }

    template <typename word_t>
    void importFromEWAH(const EWAHBoolArray<word_t>& ewahBitmap)
    {
        size_t pointer(0);
        size_t pos(0);
        const size_t wordBitNum = sizeof(word_t) << 3;
        const std::vector<word_t>& buffer = ewahBitmap.getBuffer();

        while (pointer < buffer.size())
        {
            ConstRunningLengthWord<word_t> rlw(buffer[pointer]);
            const size_t rlen = rlw.getRunningLength() * wordBitNum;

            if (rlw.getRunningBit())
            {
                for (size_t bi = 0; bi < rlen; ++bi)
                {
                    set(pos + bi);
                }
            }

            pos += rlen;
            ++pointer;

            const size_t literalWordNum = rlw.getNumberOfLiteralWords();
            for (size_t wi = 0; wi < literalWordNum; ++wi, ++pointer)
            {
                const word_t currentWord = buffer[pointer];
                word_t mask = 1;
                for (size_t bi = 0; bi < wordBitNum; ++bi, ++pos)
                {
                    if (currentWord & mask)
                    {
                        set(pos);
                    }
                    mask <<= 1;
                }
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& output, const Bitset& bv)
    {
        output << "[" << bv.size() << "] ";
        for (std::size_t i = 0; i < bv.size(); ++i)
        {
            output << (int)bv.test(i);
        }
        return output;
    }

private:
    /**
     * Get the number of bytes to contain @p pos bits.
     * @param pos the number of bits
     * @return the number of bytes
     */
    inline size_t block_num(size_t pos) const
    {
        return (pos + 63) / 64;
    }

    /**
     * Clear the dirty bits which positions are not less than @c size_.
     * For example, if the @c bits_ content is "11111111", and size_ is 4,
     * after calling this function, the last four dirty bits would be cleared,
     * that is, the @c bits content would be "11110000".
     */
    void clear_dirty_bits();

    /**
     * Grow @p bits_ to contain @p size bits.
     * @param size the number of bits
     */
    void grow(size_t size);

    template <class T>
    static inline T* getAlignedArray(size_t size)
    {
        T* block;
        if (posix_memalign((void**)&block, 64, size * sizeof(T)))
            block = (T*)malloc(size * sizeof(T));
        memset(block, 0, size * sizeof(T));
        return block;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version)  const
    {
        ar & size_;
        ar & capacity_;
        if (bits_)
        {
            ar.save_binary(bits_.get(), block_num(size_) * sizeof(uint64_t));
        }
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & size_;
        size_t capacity = 0;
        ar & capacity;
        if (capacity != capacity_)
        {
            uint64_t* newBits = getAlignedArray<uint64_t>(capacity);
            bits_.reset(newBits);
            capacity_ = capacity;
        }
        ar.load_binary(bits_.get(), block_num(size_) * sizeof(uint64_t));
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
    static const size_t npos = (size_t)-1;

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
        INIT_SIZE = 10 * 1024 * 1024,
    };

private:
    size_t size_; ///< the number of bits
    size_t capacity_;
    boost::shared_array<uint64_t> bits_;
};

}

NS_IZENELIB_IR_END

namespace std
{
    template<> inline void swap<izenelib::ir::indexmanager::Bitset>(
            izenelib::ir::indexmanager::Bitset& a,
            izenelib::ir::indexmanager::Bitset& b)
    {
        a.swap(b);
    }
}

#endif

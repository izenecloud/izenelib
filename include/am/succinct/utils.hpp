#ifndef _IZENELIB_AM_SUCCINCT_UTILS_HPP
#define _IZENELIB_AM_SUCCINCT_UTILS_HPP

#include "constants.hpp"

#include <glog/logging.h>
#include <boost/shared_ptr.hpp>

#ifndef NDEBUG
  #define __assert(x) CHECK(x)
#else
  #define __assert(x)
#endif


NS_IZENELIB_AM_BEGIN

namespace succinct
{

class SuccinctUtils
{
public:
    static inline size_t popcount(uint64_t b)
    {
        return __builtin_popcountl(b);
    }

    static inline size_t log2(uint64_t n)
    {
        return 64 - __builtin_clzl(n);
    }

    static uint64_t GetSlice(const std::vector<uint64_t>& bits,
                             size_t pos, size_t len)
    {
        if (len == 0) return 0;
        size_t block = pos / kBlockSize;
        size_t offset = pos % kBlockSize;
        uint64_t ret = bits[block] >> offset;
        if (offset + len > kBlockSize)
        {
            ret |= bits[block + 1] << (kBlockSize - offset);
        }
        if (len == 64) return ret;
        return ret & ((1LLU << len) - 1);
    }

    static void SetSlice(std::vector<uint64_t>& bits,
                         size_t pos, size_t len, uint64_t val)
    {
        if (len == 0) return;
        size_t block = pos / kBlockSize;
        size_t offset = pos % kBlockSize;
        bits[block] |= val << offset;
        if (offset + len > kBlockSize)
        {
            bits[block + 1] |= val >> (kBlockSize - offset);
        }
    }

    static inline size_t Ceiling(size_t num, size_t div)
    {
        return (num + div - 1) / div;
    }

    static size_t selectBlock(uint64_t blk, size_t r)
    {
        __assert(r < kBlockSize);

        uint64_t s = blk - ((blk >> 1) & 0x5555555555555555LLU);
        s = (s & 0x3333333333333333LLU) + ((s >> 2) & 0x3333333333333333LLU);
        s = (s + (s >> 4)) & 0x0f0f0f0f0f0f0f0fLLU;
        s *= 0x0101010101010101LLU;

        __assert(r < s >> 56);

        size_t nblock = __builtin_ctzl((s + (0x7fLLU - r) * 0x0101010101010101LLU) & 0x8080808080808080LLU) - 7;

        return nblock + kSelectPos_[r - (s << 8 >> nblock & 0xffLLU)][blk >> nblock & 0xffLLU];
    }

    template <class T>
    static void saveVec(std::ostream& os, const std::vector<T>& vs)
    {
        size_t size = vs.size();
        os.write((const char*)&size, sizeof(size));
        os.write((const char*)&vs[0], sizeof(vs[0]) * size);
    }

    template <class T>
    static void loadVec(std::istream& is, std::vector<T>& vs)
    {
        size_t size = 0;
        is.read((char*)&size, sizeof(size));
        vs.resize(size);
        is.read((char*)&vs[0], sizeof(vs[0]) * size);
    }

private:
    static const uint8_t kSelectPos_[8][256];
};

}

NS_IZENELIB_AM_END

#endif

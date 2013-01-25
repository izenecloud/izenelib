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
    static inline size_t popcount64(uint64_t b)
    {
        return __builtin_popcountll(b);
    }

    static uint64_t GetSlice(const std::vector<uint64_t>& bits,
                             size_t pos, size_t len)
    {
        if (len == 0) return 0;
        size_t block = pos / kSmallBlockSize;
        size_t offset = pos % kSmallBlockSize;
        uint64_t ret = bits[block] >> offset;
        if (offset + len > kSmallBlockSize)
        {
            ret |= bits[block + 1] << (kSmallBlockSize - offset);
        }
        if (len == 64) return ret;
        return ret & ((1LLU << len) - 1);
    }

    static void SetSlice(std::vector<uint64_t>& bits,
                         size_t pos, size_t len, uint64_t val)
    {
        if (len == 0) return;
        size_t block = pos / kSmallBlockSize;
        size_t offset = pos % kSmallBlockSize;
        bits[block] |= val << offset;
        if (offset + len > kSmallBlockSize)
        {
            bits[block + 1] |= val >> (kSmallBlockSize - offset);
        }
    }

    static inline size_t Ceiling(size_t num, size_t div)
    {
        return (num + div - 1) / div;
    }

    static size_t selectBlock(uint64_t blk, size_t r)
    {
        __assert(r < kSmallBlockSize);

        for (size_t nblock = 0; nblock < kSmallBlockSize; nblock += 8)
        {
            size_t cnt = popcount64(blk >> nblock & 0xff);

            if (r < cnt)
            {
                return nblock + kSelectPos_[r][blk >> nblock & 0xff];
            }
            else
            {
                r -= cnt;
            }
        }

        __assert(false);
        return kSmallBlockSize;
    }

#if defined(__GNUC__) && __GNUC_PREREQ(2, 2)
#define __USE_POSIX_MEMALIGN__
#endif

#ifdef __USE_POSIX_MEMALIGN__
    static uint64_t *cachealign_alloc(size_t size)
    {
        __assert(size != 0);
        uint64_t *p;
        /* NOTE: *lev2 is 64B-aligned so as to avoid cache-misses */
        int ret = posix_memalign((void **)&p, CACHELINE_SZ, size);
        return ret == 0 ? p : NULL;
    }

    static void cachealign_free(uint64_t *p)
    {
        if (p) free(p);
    }
#else
    static uint64_t *cachealign_alloc(size_t size)
    {
        __assert(size != 0);
        /* FIXME: *lev2 NEEDS to be 64B-aligned. */
        uint64_t *p = new uint64_t[size + CACHELINE_SZ];
        return p;
    }

    static void cachealign_free(uint64_t *p)
    {
        if (p) delete[] p;
    }
#endif /* __USE_POSIX_MEMALIGN__ */

private:
    static const uint8_t kSelectPos_[8][256];
};

}

NS_IZENELIB_AM_END

#endif

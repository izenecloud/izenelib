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

    static inline size_t Ceiling(size_t num, size_t div)
    {
        return (num + div - 1) / div;
    }

    static uint64_t GetSlice(const std::vector<uint64_t>& bits,
                             size_t pos, size_t len);
    static void SetSlice(std::vector<uint64_t>& bits,
                         size_t pos, size_t len, uint64_t val);

    static size_t selectBlock(uint64_t blk, size_t r);

    static uint64_t bitReverse(uint64_t v, size_t bit_num);
    static uint32_t bitReverse(uint32_t v, size_t bit_num);
    static uint16_t bitReverse(uint16_t v, size_t bit_num);

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
    static const uint8_t kBitReverseTable_[256];
};

}

NS_IZENELIB_AM_END

#endif

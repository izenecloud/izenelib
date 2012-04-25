#include <am/succinct/fujimap/fujimap_common.hpp>
#include <cassert>
#include <iostream>
using namespace std;

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

/*
0 -> 0
1 -> 1   1
2 -> 10  2
3 -> 11  2
4 -> 100 3
*/

uint64_t log2(uint64_t x)
{
    if (x == 0) return 0;
    uint64_t ret = 1;
    while (x >> ret && ret < 64)
    {
        ++ret;
    }
    if (ret == 64)
    {
        cerr << "x:" << x << endl;
    }
    return ret;
}

uint64_t gammaLen(uint64_t x)
{
    x++;
    assert(x > 0);
    return log2(x)*2 - 1;
}

uint64_t mask(uint64_t x, uint64_t len)
{
    return x & ((1LLU << len) - 1);
}

uint64_t gammaEncodeBit(uint64_t pos, uint64_t x)
{
    x++;
    assert(x > 0);
    uint64_t flagLen = log2(x) - 1;
    assert(pos < 2 * log2(x) - 1);
    if      (pos < flagLen ) return 0;
    else if (pos == flagLen) return 1;
    else                     return (x >> (pos - flagLen - 1)) & (1LLU);
}

uint64_t gammaDecode(uint64_t code)
{
    uint64_t flagPos = 0;
    while (((code >> flagPos) & 1LLU) == 0 && flagPos < 64)
    {
        ++flagPos;
    }
    if (flagPos == 64) return NOTFOUND;
    return  mask(code >> (flagPos+1), flagPos) + (1LLU << flagPos) - 1;
}

void printBit(const uint64_t v, const uint64_t len)
{
    assert(len < 64);
    for (uint64_t i = 0; i < len; ++i)
    {
        cerr << ((v >> i) & 1LLU);
        if ((i+1) % 8 == 0)
        {
            cerr << " " ;
        }
    }
    cerr << endl;
}



}}

NS_IZENELIB_AM_END


#ifndef DFMI_UTILS_H
#define DFMI_UTILS_H

#include <types.h>


NS_IZENELIB_AM_BEGIN

namespace succinct
{
namespace dfmi
{

bool isPowerOfTwo(int n);

/**
 * @return the binary logarithm of an integer.
 */
uint32_t log2(uint64_t i);

uint64_t getChrono();

}
}

NS_IZENELIB_AM_END

#endif

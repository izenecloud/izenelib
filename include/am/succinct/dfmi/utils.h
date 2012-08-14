#ifndef DFMI_UTILS_H
#define DFMI_UTILS_H

#include "types.h"


namespace dfmi
{

bool isPowerOfTwo(int n);

/**
 * @return the binary logarithm of an integer.
 */
uint32_t log2(uint64_t i);

uint64_t getChrono();

}

#endif

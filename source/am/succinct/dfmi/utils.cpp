#include <am/succinct/dfmi/utils.h>

#include <iostream>
#include <sys/time.h>


namespace dfmi
{

bool isPowerOfTwo(int n)
{
  return (n) && !(n & (n - 1));
}

/**
 * @return the binary logarithm of an integer.
 */
uint32_t log2 (uint64_t i)
{
    uint32_t size_in_bits = 8 * sizeof(i);
    uint64_t res;
    bool match = 0;
    uint32_t min = 0, max = size_in_bits - 1, shift = (min + max) / 2;

    /* binary searching the highest bit */
    match = 0;
    res = i;
    while (!match)
    {
        switch (res >> shift)
        {
        case 0:  max = shift - 1; shift = (max + min) / 2; break;
        case 1:  match = 1; break;
        default: min = shift + 1; shift = (max + min) / 2; break;
        }
    }
    return shift;
}

uint64_t getChrono()
{
    struct timeval time;
    time_t sec;
    suseconds_t usec;

    if (gettimeofday(&time, NULL) != -1)
    {
        sec = time.tv_sec;
        usec = time.tv_usec;
        return (uint64_t)sec * 1000000 + usec;
    }
    return 0;
}

}

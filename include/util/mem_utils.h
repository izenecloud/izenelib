#ifndef IZENELIB_UTIL_MEM_UTILS_H
#define IZENELIB_UTIL_MEM_UTILS_H

#include <stdlib.h>

#define CACHELINE_SIZE 64

template <class T>
static T *cachealign_alloc(size_t size, size_t alignment = CACHELINE_SIZE)
{
    assert(size != 0);
    T *p;
    int ret = posix_memalign((void **)&p, alignment, size * sizeof(T));
    return ret == 0 ? p : NULL;
}

struct cachealign_deleter
{
    void operator()(void *p) const
    {
        if (p) free(p);
    }
};

#endif

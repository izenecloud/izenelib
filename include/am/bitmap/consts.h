#ifndef IZENELIB_AM_BITMAP_CONSTS_H
#define IZENELIB_AM_BITMAP_CONSTS_H

#include <types.h>

NS_IZENELIB_AM_BEGIN

static const uint32_t MAX_CHUNK_CAPACITY = 1 << 16;
static const uint32_t BITMAP_SIZE = MAX_CHUNK_CAPACITY / 32;
static const uint32_t MAX_ARRAY_SIZE = 4096;
static const uint32_t INIT_ARRAY_SIZE = 8;

NS_IZENELIB_AM_END

#endif

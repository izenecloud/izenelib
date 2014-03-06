#ifndef _IZENELIB_AM_SUCCINCT_CONSTANTS_HPP
#define _IZENELIB_AM_SUCCINCT_CONSTANTS_HPP

#include <types.h>

NS_IZENELIB_AM_BEGIN

namespace succinct
{

#ifndef UINT8_MAX
#define UINT8_MAX (255U)
#endif

/* Defined by BSIZE */
typedef uint64_t block_t;

/* Block size (64-bit environment) */
static const size_t BSIZE = 64;
static const size_t PRESUM_SZ = 128;
static const size_t LEVEL1_NUM = 256;
static const size_t LEVEL2_NUM = BSIZE;

static const size_t kBlockSize = 64;
static const size_t kBlockPerLargeBlock = 16;
static const size_t kLargeBlockSize = kBlockSize * kBlockPerLargeBlock;
static const size_t kBlockPerSuperBlock = 6;
static const size_t kSuperBlockSize = kBlockSize * kBlockPerSuperBlock;
static const size_t kSelectBlockSize = 2048;
static const size_t kSelectLinearFallback = 16;

}

NS_IZENELIB_AM_END

#endif

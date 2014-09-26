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
static constexpr size_t BSIZE = 64;
static constexpr size_t PRESUM_SZ = 128;
static constexpr size_t LEVEL1_NUM = 256;
static constexpr size_t LEVEL2_NUM = BSIZE;

static constexpr size_t kBlockSize = 64;
static constexpr size_t kBlockPerLargeBlock = 16;
static constexpr size_t kLargeBlockSize = kBlockSize * kBlockPerLargeBlock;
static constexpr size_t kBlockPerSuperBlock = 6;
static constexpr size_t kSuperBlockSize = kBlockSize * kBlockPerSuperBlock;
static constexpr size_t kSelectBlockSize = 2048;
static constexpr size_t kSelectLinearFallback = 16;

}

NS_IZENELIB_AM_END

#endif

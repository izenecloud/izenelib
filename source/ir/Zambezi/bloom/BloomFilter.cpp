#include <ir/Zambezi/bloom/BloomFilter.hpp>
#include <ir/Zambezi/Consts.hpp>
#include <ir/Zambezi/Utils.hpp>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

uint32_t BloomFilter::computeLength(uint32_t df, uint32_t bitsPerElement)
{
    return (df * bitsPerElement + BLOOM_FILTER_UNIT_SIZE - 1) / BLOOM_FILTER_UNIT_SIZE;
}

void BloomFilter::insert(uint32_t* filter, uint32_t filterSize, uint32_t nbHash, uint32_t value)
{
    uint32_t seed = DEFAULT_HASH_SEED;
    filterSize *= BLOOM_FILTER_UNIT_SIZE;
    for (uint32_t i = 0; i < nbHash; i++)
    {
        seed = hash(value, seed);
        uint32_t h = seed % filterSize;
        filter[h / BLOOM_FILTER_UNIT_SIZE] |= 1U << (h % BLOOM_FILTER_UNIT_SIZE);
    }
}

bool BloomFilter::contains(const uint32_t* filter, uint32_t filterSize, uint32_t nbHash, uint32_t value)
{
    uint32_t seed = DEFAULT_HASH_SEED;
    filterSize *= BLOOM_FILTER_UNIT_SIZE;
    for (uint32_t i = 0; i < nbHash; i++)
    {
        seed = hash(value, seed);
        uint32_t h = seed % filterSize;
        if (!(filter[h / BLOOM_FILTER_UNIT_SIZE] & 1U << (h % BLOOM_FILTER_UNIT_SIZE)))
            return false;
    }

    return true;
}

}

NS_IZENELIB_IR_END

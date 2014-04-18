#ifndef IZENELIB_IR_ZAMBEZI_BLOOM_FILTER_HPP
#define IZENELIB_IR_ZAMBEZI_BLOOM_FILTER_HPP

#include <types.h>

NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

class BloomFilter
{
public:
    /**
     * Given the number of documents and bits per element parameter,
     * calculate the length of the Bloom filter
     *
     * @param df Document frequency
     * @param bitsPerElement Number of bits per element
     */
    static uint32_t computeLength(uint32_t df, uint32_t bitsPerElement);

    /**
     * Insert a document id into the Bloom filter
     *
     * @param filter Bloom filter
     * @param filterSize Size of the filter in number of ints
     * @param nbHash number of hash functions
     * @param value Document id
     */
    static void insert(uint32_t* filter, uint32_t filterSize, uint32_t nbHash, uint32_t value);

    /**
     * Perform a membership test on a Bloom filter
     */
    static bool contains(const uint32_t* filter, uint32_t filterSize, uint32_t nbHash, uint32_t value);
};

}

NS_IZENELIB_IR_END

#endif

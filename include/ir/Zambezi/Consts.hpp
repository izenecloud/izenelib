#ifndef IZENELIB_IR_ZAMBEZI_CONSTS_HPP
#define IZENELIB_IR_ZAMBEZI_CONSTS_HPP

#include <types.h>

#include <limits>


NS_IZENELIB_IR_BEGIN

namespace Zambezi
{

static const uint32_t BLOCK_SIZE = 128;
static const uint32_t BP_BLOCK_SIZE = 16 * BLOCK_SIZE;
static const uint32_t MAX_BLOCK_SIZE = 32 * BLOCK_SIZE;

// Pool size
static const size_t MAX_POOL_SIZE = 1ULL << 30; // 4GiB memory in default
// Number of pools in segment pool
static const uint32_t NUMBER_OF_POOLS = 16;
static const size_t HUGEPAGE_SIZE = 1ULL << 21; // 2MiB

// Document Frequency cutoff
static const uint32_t DF_CUTOFF = 16;
// Buffer expansion rate for buffer maps
static const uint32_t EXPANSION_RATE = 2;

// Null pointers to determine the end of a postings list
static const size_t UNDEFINED_POINTER = std::numeric_limits<size_t>::max();
static const uint32_t UNDEFINED_SEGMENT = std::numeric_limits<uint32_t>::max();
static const uint32_t UNDEFINED_OFFSET = std::numeric_limits<uint32_t>::max();
static const uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();

static const uint32_t BLOOM_FILTER_UNIT_SIZE = sizeof(uint32_t) * 8;
static const uint32_t DEFAULT_HASH_SEED = 0x7ed55d16;

// Default vocabulary size
static const uint32_t DEFAULT_VOCAB_SIZE = 1U << 25; // 32M terms in default
// Default number of documents in the collection
static const uint32_t DEFAULT_COLLECTION_SIZE = 30000000;

static const float DEFAULT_K1 = 0.5f;
static const float DEFAULT_B = 0.3f;

// Index type (non-positional, docids and tf, and positional)
enum IndexType
{
    NON_POSITIONAL = 0,
    TF_ONLY = 1,
    POSITIONAL = 2
};

enum Algorithm
{
    SVS = 0, // Conjunctive query evaluation using SvS
    WAND = 1, // Disjunctive query evaluation using WAND
    MBWAND = 2, // Disjunctive query evaluation using WAND_IDF
    BWAND_OR = 3, // Disjunctive BWAND
    BWAND_AND = 4, // Conjunctive BWAND
};

}

NS_IZENELIB_IR_END

#endif

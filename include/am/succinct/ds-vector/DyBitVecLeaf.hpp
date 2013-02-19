/*
 *  Copyright (c) 2012 Daisuke Okanohara
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#ifndef PREFIX_SUM_DY_BITVEC_LEAF_HPP_
#define PREFIX_SUM_DY_BITVEC_LEAF_HPP_

#include <iostream>
#include "PrefixSumLeaf.hpp"

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

struct DyBitVecLeaf
{
    static const uint64_t BLOCKSIZE = 64;
    static const uint64_t HALF_BLOCKSIZE = BLOCKSIZE / 2;
public:
    DyBitVecLeaf();
    ~DyBitVecLeaf();

    void Clear();
    void Insert(uint64_t ind, uint64_t bit);
    uint64_t Lookup(uint64_t ind) const;
    uint64_t Rank(uint64_t ind) const;
    void Split(DyBitVecLeaf* leaf);
    uint64_t InsertAndRank(uint64_t ind, uint64_t bit);
    bool IsFull() const
    {
        return sizes_.Num() == BLOCKSIZE;
    }
    uint64_t Size() const
    {
        return sizes_.Sum();
    }
    uint64_t OneNum() const
    {
        return one_nums_.Sum();
    }

private:
    void InsertAtBlock(uint64_t block_ind, uint64_t offset, uint64_t bit);
    void SplitAtBlock(uint64_t block_ind, uint64_t offset, uint64_t bit);
    bool TryMove(uint64_t block_ind, uint64_t offset, uint64_t bit);
    PrefixSumLeaf sizes_;
    PrefixSumLeaf one_nums_;
    uint64_t bits_[BLOCKSIZE];
};

}
}

NS_IZENELIB_AM_END

#endif // PREFIX_SUM_DY_BITVEC_LEAF_HPP_

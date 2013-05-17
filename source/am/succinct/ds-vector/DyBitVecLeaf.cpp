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

#include <cassert>
#include <am/succinct/ds-vector/Util.hpp>
#include <am/succinct/ds-vector/DyBitVecLeaf.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

DyBitVecLeaf::DyBitVecLeaf()
{
    for (uint64_t i = 0; i < BLOCKSIZE; ++i)
    {
        bits_[i] = 0LLU;
    }
}

DyBitVecLeaf::~DyBitVecLeaf()
{
}

void DyBitVecLeaf::Clear()
{
    sizes_.Clear();
    one_nums_.Clear();
    for (uint64_t i = 0; i < BLOCKSIZE; ++i)
    {
        bits_[i] = 0LLU;
    }
}

void DyBitVecLeaf::InsertAtBlock(uint64_t block_ind, uint64_t offset, uint64_t bit)
{
    sizes_.Increment(block_ind, 1);
    if (bit)
    {
        one_nums_.Increment(block_ind, 1);
    }
    Util::Insert(bits_[block_ind], offset, bit);
}

void DyBitVecLeaf::SplitAtBlock(uint64_t block_ind, uint64_t offset, uint64_t bit)
{
    uint64_t block_num = sizes_.Num();
    assert(block_num < BLOCKSIZE);
    for (uint64_t i = block_num; i > block_ind+1; --i)
    {
        bits_[i] = bits_[i-1];
    }
    bits_[block_ind+1] = bits_[block_ind] >> HALF_BLOCKSIZE;
    bits_[block_ind] &= ((1LLU << HALF_BLOCKSIZE) - 1);
    sizes_.Insert(block_ind, HALF_BLOCKSIZE);
    sizes_.Set(block_ind+1, HALF_BLOCKSIZE);
    one_nums_.Insert(block_ind, Util::PopCount(bits_[block_ind]));
    one_nums_.Set(block_ind+1,  Util::PopCount(bits_[block_ind+1]));

    if (offset < HALF_BLOCKSIZE)
    {
        InsertAtBlock(block_ind, offset, bit);
    }
    else
    {
        InsertAtBlock(block_ind+1, offset - HALF_BLOCKSIZE, bit);
    }
}

bool DyBitVecLeaf::TryMove(uint64_t block_ind, uint64_t offset, uint64_t bit)
{
    if (block_ind+1 < sizes_.Num() &&
            sizes_.Get(block_ind+1) < BLOCKSIZE)
    {
        uint64_t last_bit = Util::GetBit(bits_[block_ind], BLOCKSIZE-1);
        InsertAtBlock(block_ind+1, 0, last_bit);
        Util::Insert(bits_[block_ind], offset, bit);
        if (last_bit && !bit)
        {
            one_nums_.Decrement(block_ind, 1);
        }
        else if (!last_bit && bit)
        {
            one_nums_.Increment(block_ind, 1);
        }
        return true;
    }
    else if (block_ind >= 1 &&
             sizes_.Get(block_ind-1) < BLOCKSIZE)
    {
        if (offset == 0)
        {
            InsertAtBlock(block_ind-1, sizes_.Get(block_ind-1), bit);
            return true;
        }
        uint64_t first_bit = Util::GetBit(bits_[block_ind], 0);

        InsertAtBlock(block_ind-1, sizes_.Get(block_ind-1), first_bit);
        bits_[block_ind] >>= 1;
        Util::Insert(bits_[block_ind], offset-1, bit);
        if (first_bit && !bit)
        {
            one_nums_.Decrement(block_ind, 1);
        }
        else if (!first_bit && bit)
        {
            one_nums_.Increment(block_ind, 1);
        }
        return true;
    }
    return false;
}

void DyBitVecLeaf::Insert(uint64_t ind, uint64_t bit)
{
    uint64_t block_ind = sizes_.Find(ind);
    if (block_ind >= sizes_.Num() &&
            block_ind > 0 &&
            sizes_.Get(block_ind-1) < BLOCKSIZE)
    {
        --block_ind;
    }
    else if (block_ind >= sizes_.Num())
    {
        sizes_.Insert(block_ind, 0);
        one_nums_.Insert(block_ind, 0);
    }
    uint64_t offset = ind - sizes_.GetPrefixSum(block_ind);
    uint64_t size = sizes_.Get(block_ind);

    if (size < BLOCKSIZE)
    {
        InsertAtBlock(block_ind, offset, bit);
    }
    else
    {
        if (!TryMove(block_ind, offset, bit))
        {
            SplitAtBlock(block_ind, offset, bit);
        }
    }
}

uint64_t DyBitVecLeaf::InsertAndRank(uint64_t ind, uint64_t bit)
{
    uint64_t block_ind = sizes_.Find(ind);
    if (block_ind >= sizes_.Num() &&
            block_ind > 0 &&
            sizes_.Get(block_ind-1) < BLOCKSIZE)
    {
        --block_ind;
    }
    else if (block_ind >= sizes_.Num())
    {
        sizes_.Insert(block_ind, 0);
        one_nums_.Insert(block_ind, 0);
    }
    uint64_t offset = ind - sizes_.GetPrefixSum(block_ind);
    uint64_t size = sizes_.Get(block_ind);

    uint64_t rank = one_nums_.GetPrefixSum(block_ind)
                    + Util::PopCount(bits_[block_ind] & ((1LLU << offset) - 1));

    if (size < BLOCKSIZE)
    {
        InsertAtBlock(block_ind, offset, bit);
    }
    else
    {
        if (!TryMove(block_ind, offset, bit))
        {
            SplitAtBlock(block_ind, offset, bit);
        }
    }
    return rank;
}


uint64_t DyBitVecLeaf::Lookup(uint64_t ind) const
{
    uint64_t block_ind = sizes_.Find(ind);
    uint64_t offset = ind - sizes_.GetPrefixSum(block_ind);
    return Util::GetBit(bits_[block_ind], offset);
}

uint64_t DyBitVecLeaf::Rank(uint64_t ind) const
{
    uint64_t block_ind = sizes_.Find(ind);
    uint64_t offset = ind - sizes_.GetPrefixSum(block_ind);
    return one_nums_.GetPrefixSum(block_ind)
           + Util::PopCount(bits_[block_ind] & ((1LLU << offset) - 1));
}

void DyBitVecLeaf::Split(DyBitVecLeaf* leaf)
{
    sizes_.Split(leaf->sizes_);
    one_nums_.Split(leaf->one_nums_);
    for (uint64_t i = 0; i < BLOCKSIZE/2; ++i)
    {
        leaf->bits_[i] = bits_[i+BLOCKSIZE/2];
        bits_[i+BLOCKSIZE/2] = 0;
    }
}

}
}

NS_IZENELIB_AM_END

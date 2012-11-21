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

#include <am/succinct/ds-vector/DyWaveletTree.hpp>
#include <am/succinct/ds-vector/Util.hpp>

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

DyWaveletTree::DyWaveletTree() : size_(0)
{
}

DyWaveletTree::~DyWaveletTree()
{
}

void DyWaveletTree::Clear()
{
    for (uint64_t i = 0; i < 0x100; ++i)
    {
        dbvs_[i].Clear();
    }
    size_ = 0;
}

void DyWaveletTree::Insert(uint64_t ind, uint8_t c)
{
    uint8_t node_ind = 1;
    for (uint64_t depth = 7; ; --depth)
    {
        uint64_t bit = (c >> depth) & 1LLU;
        ind = dbvs_[node_ind].InsertAndRank(ind, bit);
        if (depth == 0) break;
        node_ind = (node_ind << 1) + bit;
    }
    ++size_;
}

uint64_t DyWaveletTree::Rank(uint64_t ind, uint8_t c) const
{
    uint8_t node_ind = 1;
    for (uint64_t depth = 8; depth > 0;)
    {
        --depth;
        uint64_t bit = (c >> depth) & 1LLU;
        ind = dbvs_[node_ind].Rank(ind, bit);
        node_ind = (node_ind << 1) + bit;
    }
    return ind;
}

uint8_t DyWaveletTree::Lookup(uint64_t ind) const
{
    uint8_t node_ind = 1;
    for (uint64_t depth = 7; ; --depth)
    {
        uint64_t bit = dbvs_[node_ind].Lookup(ind);
        if (depth == 0)
        {
            return (node_ind << 1) + bit;
        }
        ind = dbvs_[node_ind].Rank(ind, bit);
        node_ind = (node_ind << 1) + bit;
    }
}

}
}

NS_IZENELIB_AM_END


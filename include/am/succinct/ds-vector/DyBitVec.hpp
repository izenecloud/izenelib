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

#ifndef PREFIX_SUM_DY_BITVEC_HPP_
#define PREFIX_SUM_DY_BITVEC_HPP_

#include <types.h>
#include "DyBitVecNode.hpp"

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

class DyBitVec
{
public:
    DyBitVec();
    ~DyBitVec();
    void Clear();
    void Insert(uint64_t ind, uint64_t bit);
    uint64_t Lookup(uint64_t ind) const;
    uint64_t Rank(uint64_t ind, uint64_t bit) const;
    uint64_t InsertAndRank(uint64_t ind, uint64_t bit);
    uint64_t LeafNum() const;

    inline uint64_t Size() const
    {
        return size_;
    }

    inline uint64_t OneNum() const
    {
        return one_num_;
    }

private:
    DyBitVecNode root_;
    uint64_t size_;
    uint64_t one_num_;
};

}
}

NS_IZENELIB_AM_END

#endif // PREFIX_SUM_DY_BITVEC_HPP_

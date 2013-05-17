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

#ifndef PREFIX_SUM_PREFIX_SUM_HPP_
#define PREFIX_SUM_PREFIX_SUM_HPP_

#include <vector>
#include <types.h>
#include "PrefixSumNode.hpp"

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

class PrefixSum
{
public:
    PrefixSum();
    ~PrefixSum();
    void Clear();
    void Insert(uint64_t ind, uint64_t val);
    void Increment(uint64_t ind, uint64_t val);
    void Decrement(uint64_t ind, uint64_t val);
    void Set(uint64_t ind, uint64_t val);
    uint64_t Get(uint64_t ind) const;
    uint64_t GetPrefixSum(uint64_t ind) const;
    uint64_t Find(uint64_t val) const;
    uint64_t Num() const
    {
        return num_;
    }
    uint64_t Sum() const
    {
        return sum_;
    }

private:
    PrefixSumNode root_;
    uint64_t num_;
    uint64_t sum_;
};


}
}

NS_IZENELIB_AM_END

#endif // PREFIX_SUM_PREFIX_SUM64_HPP_

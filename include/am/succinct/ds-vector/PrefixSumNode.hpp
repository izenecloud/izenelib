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

#ifndef PREFIX_SUM_PREFIX_SUM_NODE_HPP_
#define PREFIX_SUM_PREFIX_SUM_NODE_HPP_

#include <types.h>
#include "PrefixSumLeaf.hpp"

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

struct PrefixSumNode
{
    PrefixSumNode();
    ~PrefixSumNode();

    void Clear();
    bool IsLeaf() const
    {
        return leaf != NULL;
    }

    uint64_t left_size;
    uint64_t left_sum;

    PrefixSumNode** children;
    PrefixSumLeaf* leaf;
};

}
}

NS_IZENELIB_AM_END

#endif // PREFIX_SUM_PREFIX_SUM_NODE_HPP_

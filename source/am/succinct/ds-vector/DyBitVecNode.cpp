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

#include <am/succinct/ds-vector/DyBitVecNode.hpp>

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

DyBitVecNode::DyBitVecNode() : left_size(0), left_one_num(0)
{
    children = NULL;
    leaf = NULL;
}

DyBitVecNode::~DyBitVecNode()
{
    if (children)
    {
        delete children[0];
        delete children[1];
    }
    delete children;
    delete leaf;
}

void DyBitVecNode::Clear()
{
    if (children)
    {
        delete children[0];
        delete children[1];
    }
    delete children;
    delete leaf;
    children = NULL;
    leaf = NULL;
    left_size = 0;
    left_one_num = 0;
}

uint64_t DyBitVecNode::LeafNum() const
{
    if (IsLeaf())
    {
        leaf->Size();
        return 1;
    }
    else return children[0]->LeafNum() +
                    children[1]->LeafNum();
}


}
}

NS_IZENELIB_AM_END

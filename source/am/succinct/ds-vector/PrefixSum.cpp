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
#include <am/succinct/ds-vector/PrefixSum.hpp>

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

namespace
{
void Split(PrefixSumNode* p)
{
    assert(p != NULL);
    PrefixSumLeaf* leaf = p->leaf;
    assert(leaf != NULL);
    PrefixSumLeaf* new_leaf = new PrefixSumLeaf;
    leaf->Split(*new_leaf);
    p->children = new PrefixSumNode* [2];
    p->children[0] = new PrefixSumNode;
    p->children[1] = new PrefixSumNode;
    p->children[0]->leaf = leaf;
    p->children[1]->leaf = new_leaf;
    p->leaf = NULL;
    p->left_size = leaf->Num();
    p->left_sum = leaf->Sum();
}

}

PrefixSum::PrefixSum() : num_(0), sum_(0)
{
    root_.leaf = new PrefixSumLeaf;
}
PrefixSum::~PrefixSum()
{
    root_.Clear();
}

void PrefixSum::Clear()
{
    root_.Clear();
    root_.leaf = new PrefixSumLeaf;
    num_ = 0;
    sum_ = 0;
}

void PrefixSum::Insert(uint64_t ind, uint64_t val)
{
    assert(ind <= num_);
    PrefixSumNode* p = &root_;
    uint64_t offset = ind;
    for (;;)
    {
        if (p->IsLeaf())
        {
            if (!p->leaf->IsFull())
            {
                break;
            }
            else
            {
                Split(p);
            }
        }
        if (offset < p->left_size)
        {
            p->left_size++;
            p->left_sum += val;
            p = p->children[0];
        }
        else
        {
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    p->leaf->Insert(offset, val);
    ++num_;
    sum_ += val;
}

void PrefixSum::Increment(uint64_t ind, uint64_t val)
{
    assert(ind < num_);
    PrefixSumNode* p = &root_;
    uint64_t offset = ind;
    while (!p->IsLeaf())
    {
        if (offset < p->left_size)
        {
            p->left_sum += val;
            p = p->children[0];
        }
        else
        {
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    p->leaf->Increment(offset, val);
    sum_ += val;
}

void PrefixSum::Decrement(uint64_t ind, uint64_t val)
{
    assert(ind < num_);
    PrefixSumNode* p = &root_;
    uint64_t offset = ind;
    while (!p->IsLeaf())
    {
        if (offset < p->left_size)
        {
            p->left_sum -= val;
            p = p->children[0];
        }
        else
        {
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    p->leaf->Decrement(offset, val);
    sum_ -= val;
}

void PrefixSum::Set(uint64_t ind, uint64_t val)
{
    uint64_t old_val = Get(ind);
    int64_t dif = val - old_val;
    assert(ind < num_);
    PrefixSumNode* p = &root_;
    uint64_t offset = ind;
    while (!p->IsLeaf())
    {
        if (offset < p->left_size)
        {
            p->left_sum += dif;
            p = p->children[0];
        }
        else
        {
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    p->leaf->Set(offset, val);
    sum_ += dif;
}

uint64_t PrefixSum::Get(uint64_t ind) const
{
    assert(ind < num_);
    const PrefixSumNode* p = &root_;
    uint64_t offset = ind;
    while (!p->IsLeaf())
    {
        if (offset < p->left_size)
        {
            p = p->children[0];
        }
        else
        {
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    assert(offset < p->leaf->Num());
    return p->leaf->Get(offset);
}

uint64_t PrefixSum::GetPrefixSum(uint64_t ind) const
{
    assert(ind <= num_);
    const PrefixSumNode* p = &root_;
    uint64_t offset = ind;
    uint64_t sum = 0;
    while (!p->IsLeaf())
    {
        if (offset < p->left_size)
        {
            p = p->children[0];
        }
        else
        {
            sum += p->left_sum;
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    assert(offset <= p->leaf->Num());
    return sum + p->leaf->GetPrefixSum(offset);
}

uint64_t PrefixSum::Find(uint64_t val) const
{
    const PrefixSumNode* p = &root_;
    uint64_t offset = 0;
    uint64_t remain = val;
    while (!p->IsLeaf())
    {
        if (remain < p->left_sum)
        {
            p = p->children[0];
        }
        else
        {
            remain -= p->left_sum;
            offset += p->left_size;
            p = p->children[1];
        }
    }
    return offset + p->leaf->Find(remain);

}

}
}

NS_IZENELIB_AM_END

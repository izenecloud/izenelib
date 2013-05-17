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
#include <am/succinct/ds-vector/DyBitVec.hpp>

using namespace std;

NS_IZENELIB_AM_BEGIN
namespace succinct
{

namespace ds_vector
{

namespace
{
void Split(DyBitVecNode* p)
{
    assert(p != NULL);
    DyBitVecLeaf* leaf = p->leaf;
    assert(leaf != NULL);
    DyBitVecLeaf* new_leaf = new DyBitVecLeaf;
    leaf->Split(new_leaf);
    p->children = new DyBitVecNode* [2];
    p->children[0] = new DyBitVecNode;
    p->children[1] = new DyBitVecNode;
    p->children[0]->leaf = leaf;
    p->children[1]->leaf = new_leaf;
    p->leaf = NULL;
    p->left_size = leaf->Size();
    p->left_one_num = leaf->OneNum();
}
}

DyBitVec::DyBitVec(): size_(0), one_num_(0)
{
    root_.leaf = new DyBitVecLeaf;
}

DyBitVec::~DyBitVec()
{
    root_.Clear();
}

void DyBitVec::Clear()
{
    root_.Clear();
    root_.leaf = new DyBitVecLeaf;
    size_ = 0;
    one_num_ = 0;
}

void DyBitVec::Insert(uint64_t ind, uint64_t bit)
{
    assert(ind <= size_);
    DyBitVecNode* p = &root_;
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
            p->left_one_num += bit;
            p = p->children[0];
        }
        else
        {
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    p->leaf->Insert(offset, bit);
    ++size_;
    one_num_ += bit;
}

uint64_t DyBitVec::Lookup(uint64_t ind) const
{
    assert(ind < size_);
    const DyBitVecNode* p = &root_;
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
    assert(offset < p->leaf->Size());
    return p->leaf->Lookup(offset);
}

uint64_t DyBitVec::Rank(uint64_t ind, uint64_t bit) const
{
    if (ind > size_) ind = size_;
    const DyBitVecNode* p = &root_;
    uint64_t offset = ind;
    uint64_t rank = 0;
    while (!p->IsLeaf())
    {
        if (offset < p->left_size)
        {
            p = p->children[0];
        }
        else
        {
            rank += p->left_one_num;
            offset -= p->left_size;
            p = p->children[1];
        }
    }
    assert(offset <= p->leaf->Size());
    rank += p->leaf->Rank(offset);
    if (bit) return rank;
    else return ind - rank;
}

uint64_t DyBitVec::InsertAndRank(uint64_t ind, uint64_t bit)
{
    assert(ind <= size_);
    DyBitVecNode* p = &root_;
    uint64_t offset = ind;
    uint64_t rank = 0;
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
            p->left_one_num += bit;
            p = p->children[0];
        }
        else
        {
            rank += p->left_one_num;
            offset -= p->left_size;
            p = p->children[1];
        }
    }

    ++size_;
    one_num_ += bit;
    rank += p->leaf->InsertAndRank(offset, bit);
    if (bit) return rank;
    else return ind - rank;
}


uint64_t DyBitVec::LeafNum() const
{
    return root_.LeafNum();
}

}
}

NS_IZENELIB_AM_END

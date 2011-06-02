//
//  boost/memory/scoped_alloc.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef BOOST_MEMORY_SCOPED_ALLOC_HPP
#define BOOST_MEMORY_SCOPED_ALLOC_HPP

#include "basic.hpp"

#include "auto_alloc.hpp"
#include <util/singleton.h>

#include <boost/assert.hpp>

#include <climits> // INT_MAX

NS_BOOST_MEMORY_BEGIN

// -------------------------------------------------------------------------
// class block_pool

template <class PolicyT>
class block_alloc
{
private:
    typedef typename PolicyT::allocator_type AllocT;
    enum { CbBlock = PolicyT::MemBlockSize };

#pragma pack(1)
    struct Block
    {
        Block* next;
    };
#pragma pack()

    Block* freeList_;

    int nFree_;
    const int nFreeLimit_;
private:
    block_alloc(const block_alloc& other);
    block_alloc& operator=(const block_alloc& other);
public:

    block_alloc(int cbFreeLimit = INT_MAX)
            : freeList_(NULL), nFree_(0),
            nFreeLimit_(cbFreeLimit / CbBlock + 1)
    {
    }
    ~block_alloc()
    {
        clear();
    }

public:
    void* allocate(size_t cb)
    {
        BOOST_ASSERT(cb >= (size_t)CbBlock);

        if (cb > (size_t)CbBlock)
            return AllocT::allocate(cb);
        else
        {
            if (freeList_)
            {
                BOOST_ASSERT(AllocT::alloc_size(freeList_) >= cb);
                Block* blk = freeList_;
                freeList_ = blk->next;
                --nFree_;
                return blk;
            }
            return AllocT::allocate(CbBlock);
        }
    }

    void deallocate(void* p)
    {
        if (nFree_ >= nFreeLimit_)
        {
            AllocT::deallocate(p);
        }
        else
        {
            Block* blk = (Block*)p;
            blk->next = freeList_;
            freeList_ = blk;
            ++nFree_;
        }
    }

    static size_t alloc_size(void* p)
    {
        return AllocT::alloc_size(p);
    }

    void clear()
    {
        while (freeList_)
        {
            Block* blk = freeList_;
            freeList_ = blk->next;
            AllocT::deallocate(blk);
        }
        nFree_ = 0;
    }

};

#pragma pack(1)

template <class AllocT>
class proxy_alloc
{
private:
  AllocT* alloc_;
public:
  proxy_alloc() : alloc_(::izenelib::util::Singleton<AllocT>::get()) {}
  proxy_alloc(AllocT& alloc) : alloc_(&alloc){}
public:
  typedef size_t size_type;
                                        
public:
  inline void* allocate(size_type cb)    { return alloc_->allocate(cb); }
  inline void deallocate(void* p) { alloc_->deallocate(p); }
  inline void swap(proxy_alloc& o) { std::swap(alloc_, o.alloc_); }
  inline size_type alloc_size(void* p) const { return alloc_->alloc_size(p); }
  inline AllocT& instance() const { return *alloc_; }
  inline AllocT* operator&() const { return alloc_; }
  inline operator AllocT&() const { return *alloc_; }
};

#pragma pack()

typedef block_alloc<NS_BOOST_MEMORY_POLICY::sys> block_pool;
typedef proxy_alloc<block_pool> proxy_block_pool;

NS_BOOST_MEMORY_POLICY_BEGIN

class pool : public sys
{
public:
    typedef proxy_block_pool allocator_type;
};

NS_BOOST_MEMORY_POLICY_END

typedef region_alloc<NS_BOOST_MEMORY_POLICY::pool> scoped_alloc;

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_SCOPED_ALLOC_HPP */

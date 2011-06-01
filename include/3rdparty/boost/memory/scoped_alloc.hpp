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
    enum { m_cbBlock = PolicyT::MemBlockSize };

#pragma pack(1)
    struct Block
    {
        Block* next;
    };
#pragma pack()

    Block* m_freeList;

    int m_nFree;
    const int m_nFreeLimit;

public:

    block_alloc(int cbFreeLimit = INT_MAX)
            : m_freeList(NULL), m_nFree(0),
            m_nFreeLimit(cbFreeLimit / m_cbBlock + 1)
    {
    }
    ~block_alloc()
    {
        clear();
    }

public:
    void* allocate(size_t cb)
    {
        BOOST_ASSERT(cb >= (size_t)m_cbBlock);

        if (cb > (size_t)m_cbBlock)
            return AllocT::allocate(cb);
        else
        {
            if (m_freeList)
            {
                BOOST_ASSERT(AllocT::alloc_size(m_freeList) >= cb);
                Block* blk = m_freeList;
                m_freeList = blk->next;
                --m_nFree;
                return blk;
            }
            return AllocT::allocate(m_cbBlock);
        }
    }

    void deallocate(void* p)
    {
        if (m_nFree >= m_nFreeLimit)
        {
            AllocT::deallocate(p);
        }
        else
        {
            Block* blk = (Block*)p;
            blk->next = m_freeList;
            m_freeList = blk;
            ++m_nFree;
        }
    }

    static size_t alloc_size(void* p)
    {
        return AllocT::alloc_size(p);
    }

    void clear()
    {
        while (m_freeList)
        {
            Block* blk = m_freeList;
            m_freeList = blk->next;
            AllocT::deallocate(blk);
        }
        m_nFree = 0;
    }
};

typedef block_alloc<NS_BOOST_MEMORY_POLICY::sys> block_pool;

NS_BOOST_MEMORY_POLICY_BEGIN

class pool : public sys
{
public:
    typedef block_pool allocator_type;
};

NS_BOOST_MEMORY_POLICY_END

typedef region_alloc<NS_BOOST_MEMORY_POLICY::pool> scoped_alloc;

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_SCOPED_ALLOC_HPP */

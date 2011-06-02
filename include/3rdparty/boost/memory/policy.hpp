//
//  boost/memory/policy.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef BOOST_MEMORY_POLICY_HPP
#define BOOST_MEMORY_POLICY_HPP

#include "system_alloc.hpp"

NS_BOOST_MEMORY_BEGIN

// -------------------------------------------------------------------------
// class simple_gc_alloc

template <class SysAllocT>
class simple_gc_alloc
{
#pragma pack(1)
private:
    struct DestroyNode
    {
        DestroyNode* pPrev;
        destructor_t fnDestroy;
    };
    struct MemBlock
    {
        MemBlock* pPrev;
    };
#pragma pack()

    MemBlock* memChain_;
    DestroyNode* destroyChain_;

public:
    simple_gc_alloc()
        : memChain_(NULL), destroyChain_(NULL)
    {
    }

    void swap(simple_gc_alloc& o)
    {
        std::swap(memChain_, o.memChain_);
        std::swap(destroyChain_, o.destroyChain_);
    }

    void clear()
    {
        while (destroyChain_)
        {
            DestroyNode* curr = destroyChain_;
            destroyChain_ = destroyChain_->pPrev;
            curr->fnDestroy(curr + 1);
        }
        while (memChain_)
        {
            MemBlock* curr = memChain_;
            memChain_ = memChain_->pPrev;
            SysAllocT::deallocate(curr);
        }
    }

    template <class Type>
    void destroy(Type* obj)
    {
        // no action
    }

    template <class Type>
    Type* newArray(size_t count, Type* zero)
    {
        Type* array = (Type*)destructor_traits<Type>::allocArray(*this, count);
        return constructor_traits<Type>::constructArray(array, count);
    }

    template <class Type>
    void destroyArray(Type* array, size_t count)
    {
        // no action
    }

    void* allocate(size_t cb)
    {
        MemBlock* p = (MemBlock*)SysAllocT::allocate(cb + sizeof(MemBlock));
        p->pPrev = memChain_;
        memChain_ = p;
        return p + 1;
    }

    void* allocate(size_t cb, destructor_t fn)
    {
        DestroyNode* pNode = (DestroyNode*)allocate(cb + sizeof(DestroyNode));
        pNode->fnDestroy = fn;
        pNode->pPrev = destroyChain_;
        destroyChain_ = pNode;
        return pNode + 1;
    }

    void* allocate(size_t cb, int fnZero)
    {
        return allocate(cb);
    }

    void deallocate(void* p, size_t cb)
    {
        // no action
    }
};

// -------------------------------------------------------------------------

#ifndef NS_BOOST_MEMORY_POLICY_BEGIN
#define NS_BOOST_MEMORY_POLICY_BEGIN	namespace policy {
#define NS_BOOST_MEMORY_POLICY_END		}
#define NS_BOOST_MEMORY_POLICY			boost::memory::policy
#endif

NS_BOOST_MEMORY_POLICY_BEGIN

class sys
{
private:
    enum { Default = 0 };

public:
    enum { MemBlockSize = BOOST_MEMORY_BLOCK_SIZE };
    typedef system_alloc allocator_type;

    enum { RecycleSizeMin = 256 };
    enum { AllocSizeBig = Default };
    enum { AllocSizeHuge = 1024*1024 };
    enum { GCLimitSizeDef = 1024*1024 };

    typedef simple_gc_alloc<system_alloc> huge_gc_allocator;
};

class stdlib : public sys
{
public:
    typedef stdlib_alloc allocator_type;
};

NS_BOOST_MEMORY_POLICY_END

// -------------------------------------------------------------------------
// $Log: policy.hpp,v $

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_POLICY_HPP */

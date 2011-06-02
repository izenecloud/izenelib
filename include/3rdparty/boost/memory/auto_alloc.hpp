//
//  boost/memory/auto_alloc.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef BOOST_MEMORY_AUTO_ALLOC_HPP
#define BOOST_MEMORY_AUTO_ALLOC_HPP

#include "policy.hpp"

#include <boost/assert.hpp>

NS_BOOST_MEMORY_BEGIN

// -------------------------------------------------------------------------
// class region_alloc

template <class PolicyT>
class region_alloc
{
private:
    typedef typename PolicyT::allocator_type AllocT;

public:
    enum { MemBlockSize = PolicyT::MemBlockSize };
    enum { IsGCAllocator = true };

    typedef AllocT allocator_type;

private:
    enum { HeaderSize = sizeof(void*) };
    enum { BlockSize = MemBlockSize - HeaderSize };

#pragma pack(1)
private:
    struct MemBlock;
    friend struct MemBlock;

    struct MemBlock
    {
        MemBlock* pPrev;
        char buffer[BlockSize];
    };
    struct DestroyNode
    {
        DestroyNode* pPrev;
        destructor_t fnDestroy;
    };
#pragma pack()

    char* begin_;
    char* end_;
    AllocT alloc_;
    DestroyNode* destroyChain_;

private:
    const region_alloc& operator=(const region_alloc&);

    MemBlock* _chainHeader() const
    {
        return (MemBlock*)(begin_ - HeaderSize);
    }

    void _init()
    {
        MemBlock* pNew = (MemBlock*)alloc_.allocate(sizeof(MemBlock));
        pNew->pPrev = NULL;
        begin_ = pNew->buffer;
        end_ = (char*)pNew + alloc_.alloc_size(pNew);
    }

public:
    region_alloc() : destroyChain_(NULL)
    {
        _init();
    }
    explicit region_alloc(AllocT alloc) : alloc_(alloc), destroyChain_(NULL)
    {
        _init();
    }
    explicit region_alloc(region_alloc& owner)
            : alloc_(owner.alloc_), destroyChain_(NULL)
    {
        _init();
    }

    ~region_alloc()
    {
        clear();
    }

    void swap(region_alloc& o)
    {
        std::swap(begin_, o.begin_);
        std::swap(end_, o.end_);
        std::swap(destroyChain_, o.destroyChain_);
        alloc_.swap(o.alloc_);
    }

    void clear()
    {
        while (destroyChain_)
        {
            DestroyNode* curr = destroyChain_;
            destroyChain_ = destroyChain_->pPrev;
            curr->fnDestroy(curr + 1);
        }
        MemBlock* pHeader = _chainHeader();
        while (pHeader)
        {
            MemBlock* curr = pHeader;
            pHeader = pHeader->pPrev;
            alloc_.deallocate(curr);
        }
        begin_ = end_ = (char*)HeaderSize;
    }

private:
    void* _do_allocate(size_t cb)
    {
        if (cb >= BlockSize)
        {
            MemBlock* pHeader = _chainHeader();
            MemBlock* pNew = (MemBlock*)alloc_.allocate(HeaderSize + cb);
            if (pHeader)
            {
                pNew->pPrev = pHeader->pPrev;
                pHeader->pPrev = pNew;
            }
            else
            {
                end_ = begin_ = pNew->buffer;
                pNew->pPrev = NULL;
            }
            return pNew->buffer;
        }
        else
        {
            MemBlock* pNew = (MemBlock*)alloc_.allocate(sizeof(MemBlock));
            pNew->pPrev = _chainHeader();
            begin_ = pNew->buffer;
            end_ = (char*)pNew + alloc_.alloc_size(pNew);
            return end_ -= cb;
        }
    }

public:
    inline void* allocate(size_t cb)
    {
        if ((size_t)(end_ - begin_) >= cb)
        {
            return end_ -= cb;
        }
        return _do_allocate(cb);
    }

    inline void* allocate(size_t cb, int fnZero)
    {
        return allocate(cb);
    }

    inline void* allocate(size_t cb, destructor_t fn)
    {
        DestroyNode* pNode = (DestroyNode*)allocate(sizeof(DestroyNode) + cb);
        pNode->fnDestroy = fn;
        pNode->pPrev = destroyChain_;
        destroyChain_ = pNode;
        return pNode + 1;
    }

    inline void* unmanaged_alloc(size_t cb, destructor_t fn)
    {
        DestroyNode* pNode = (DestroyNode*)allocate(sizeof(DestroyNode) + cb);
        pNode->fnDestroy = fn;
        return pNode + 1;
    }

    inline void* manage(void* p, destructor_t fn)
    {
        DestroyNode* pNode = (DestroyNode*)p - 1;
        BOOST_ASSERT(pNode->fnDestroy == fn);

        pNode->pPrev = destroyChain_;
        destroyChain_ = pNode;
        return p;
    }

    inline void* unmanaged_alloc(size_t cb, int fnZero)
    {
        return allocate(cb);
    }

    inline void* manage(void* p, int fnZero)
    {
        return p;
    }

    void* reallocate(void* p, size_t oldSize, size_t newSize)
    {
        if (oldSize >= newSize)
            return p;
        void* p2 = allocate(newSize);
        memcpy(p2, p, oldSize);
        return p2;
    }

    void deallocate(void* p, size_t cb)
    {
        // no action
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
};

// -------------------------------------------------------------------------
// class auto_alloc

typedef region_alloc<NS_BOOST_MEMORY_POLICY::stdlib> auto_alloc;

// -------------------------------------------------------------------------
// $Log: auto_alloc.hpp,v $

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_AUTO_ALLOC_HPP */

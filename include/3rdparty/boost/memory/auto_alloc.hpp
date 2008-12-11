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

#ifndef BOOST_MEMORY_SYSTEM_ALLOC_HPP
#include "system_alloc.hpp"
#endif

#ifndef BOOST_MEMORY_POLICY_HPP
#include "policy.hpp"
#endif

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
	enum { IsGCAllocator = TRUE };

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
	
	char* m_begin;
	char* m_end;
	AllocT m_alloc;
	DestroyNode* m_destroyChain;

private:
	const region_alloc& operator=(const region_alloc&);

	MemBlock* BOOST_MEMORY_CALL _chainHeader() const
	{
		return (MemBlock*)(m_begin - HeaderSize);
	}

	void BOOST_MEMORY_CALL _init()
	{
		MemBlock* pNew = (MemBlock*)m_alloc.allocate(sizeof(MemBlock));
		pNew->pPrev = NULL;
		m_begin = pNew->buffer;
		m_end = (char*)pNew + m_alloc.alloc_size(pNew);
	}

public:
	region_alloc() : m_destroyChain(NULL)
	{
		_init();
	}
	explicit region_alloc(AllocT alloc) : m_alloc(alloc), m_destroyChain(NULL)
	{
		_init();
	}
	explicit region_alloc(region_alloc& owner)
		: m_alloc(owner.m_alloc), m_destroyChain(NULL)
	{
		_init();
	}

	~region_alloc()
	{
		clear();
	}

	void BOOST_MEMORY_CALL swap(region_alloc& o)
	{
		std::swap(m_begin, o.m_begin);
		std::swap(m_end, o.m_end);
		std::swap(m_destroyChain, o.m_destroyChain);
		m_alloc.swap(o.m_alloc);
	}

	void BOOST_MEMORY_CALL clear()
	{
		while (m_destroyChain)
		{
			DestroyNode* curr = m_destroyChain;
			m_destroyChain = m_destroyChain->pPrev;
			curr->fnDestroy(curr + 1);
		}
		MemBlock* pHeader = _chainHeader();
		while (pHeader)
		{
			MemBlock* curr = pHeader;
			pHeader = pHeader->pPrev;
			m_alloc.deallocate(curr);
		}
		m_begin = m_end = (char*)HeaderSize;
	}

private:
	void* BOOST_MEMORY_CALL _do_allocate(size_t cb)
	{
		if (cb >= BlockSize)
		{
			MemBlock* pHeader = _chainHeader();
			MemBlock* pNew = (MemBlock*)m_alloc.allocate(HeaderSize + cb);
			if (pHeader)
			{
				pNew->pPrev = pHeader->pPrev;
				pHeader->pPrev = pNew;
			}
			else
			{
				m_end = m_begin = pNew->buffer;
				pNew->pPrev = NULL;
			}
			return pNew->buffer;
		}
		else
		{
			MemBlock* pNew = (MemBlock*)m_alloc.allocate(sizeof(MemBlock));
			pNew->pPrev = _chainHeader();
			m_begin = pNew->buffer;
			m_end = (char*)pNew + m_alloc.alloc_size(pNew);
			return m_end -= cb;
		}
	}

public:
	__forceinline void* BOOST_MEMORY_CALL allocate(size_t cb)
	{
		if ((size_t)(m_end - m_begin) >= cb)
		{
			return m_end -= cb;
		}
		return _do_allocate(cb);
	}

	__forceinline void* BOOST_MEMORY_CALL allocate(size_t cb, int fnZero)
	{
		return allocate(cb);
	}

	__forceinline void* BOOST_MEMORY_CALL allocate(size_t cb, destructor_t fn)
	{
		DestroyNode* pNode = (DestroyNode*)allocate(sizeof(DestroyNode) + cb);
		pNode->fnDestroy = fn;
		pNode->pPrev = m_destroyChain;
		m_destroyChain = pNode;
		return pNode + 1;
	}

	__forceinline void* BOOST_MEMORY_CALL unmanaged_alloc(size_t cb, destructor_t fn)
	{
		DestroyNode* pNode = (DestroyNode*)allocate(sizeof(DestroyNode) + cb);
		pNode->fnDestroy = fn;
		return pNode + 1;
	}

	__forceinline void* BOOST_MEMORY_CALL manage(void* p, destructor_t fn)
	{
		DestroyNode* pNode = (DestroyNode*)p - 1;
		BOOST_MEMORY_ASSERT(pNode->fnDestroy == fn);

		pNode->pPrev = m_destroyChain;
		m_destroyChain = pNode;
		return p;
	}

	__forceinline void* BOOST_MEMORY_CALL unmanaged_alloc(size_t cb, int fnZero)
	{
		return allocate(cb);
	}

	__forceinline void* BOOST_MEMORY_CALL manage(void* p, int fnZero)
	{
		return p;
	}

	void* BOOST_MEMORY_CALL reallocate(void* p, size_t oldSize, size_t newSize)
	{
		if (oldSize >= newSize)
			return p;
		void* p2 = allocate(newSize);
		memcpy(p2, p, oldSize);
		return p2;
	}

	void BOOST_MEMORY_CALL deallocate(void* p, size_t cb)
	{
		// no action
	}

	template <class Type>
	void BOOST_MEMORY_CALL destroy(Type* obj)
	{
		// no action
	}

	template <class Type>
	Type* BOOST_MEMORY_CALL newArray(size_t count, Type* zero)
	{
		Type* array = (Type*)destructor_traits<Type>::allocArray(*this, count);
		return constructor_traits<Type>::constructArray(array, count);
	}

	template <class Type>
	void BOOST_MEMORY_CALL destroyArray(Type* array, size_t count)
	{
		// no action
	}

	BOOST_MEMORY_FAKE_DBG_ALLOCATE_();
};

// -------------------------------------------------------------------------
// class auto_alloc

typedef region_alloc<NS_BOOST_MEMORY_POLICY::stdlib> auto_alloc;

// -------------------------------------------------------------------------
// $Log: auto_alloc.hpp,v $

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_AUTO_ALLOC_HPP */

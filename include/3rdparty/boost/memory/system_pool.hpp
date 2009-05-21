//
//  boost/memory/system_pool.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef __BOOST_MEMORY_SYSTEM_POOL_HPP__
#define __BOOST_MEMORY_SYSTEM_POOL_HPP__
#include "basic.hpp"



NS_BOOST_MEMORY_BEGIN

// -------------------------------------------------------------------------
// class normal_stack

class normal_stack
{
private:
	normal_stack(const normal_stack&);
	void operator=(const normal_stack&);

	typedef NS_BOOST_DETAIL::default_threadmodel::cs cs;
	typedef cs::scoped_lock cslock;

public:
	class node
	{
	public:
		node* _m_prev;
		node* prev() const { return _m_prev; }
#if defined(_DEBUG)
		node() : _m_prev(NULL) {}
		void prev(node* p) { _m_prev = p; }
#endif
	};

private:
	node* m_top;
	cs m_cs;

public:
	normal_stack() : m_top(NULL) {}

	void BOOST_MEMORY_CALL push(node* val)
	{
		cslock aLock(m_cs);
		val->_m_prev = m_top;
		m_top = val;
	}

	node* BOOST_MEMORY_CALL clear()
	{
		cslock aLock(m_cs);
		node* the_top = m_top;
		m_top = NULL;
		return the_top;
	}

	node* BOOST_MEMORY_CALL pop()
	{
		cslock aLock(m_cs);
		node* the_top = m_top;
		if (the_top == NULL)
			return NULL;
		m_top = m_top->_m_prev;
		return the_top;
	}
};

typedef normal_stack default_stack;

// -------------------------------------------------------------------------
// class system_pool_imp

//template <class PolicyT, class StackT = default_stack>
template <class AllocT, class StackT = default_stack>

class system_pool_imp
{
private:
	//typedef typename PolicyT::allocator_type AllocT;
	//enum { cbBlock = PolicyT::MemBlockSize };
	enum { cbBlock = BOOST_MEMORY_BLOCK_SIZE};
	typedef typename StackT::node Block;
	StackT m_freeList;

private:
	system_pool_imp(const system_pool_imp&);
	void operator=(const system_pool_imp&);

public:
	system_pool_imp()
	{
	}
	~system_pool_imp()
	{
		clear();
	}

public:
	void* BOOST_MEMORY_CALL allocate(size_t cb)
	{
		BOOST_MEMORY_ASSERT(cb >= (size_t)cbBlock);

		if (cb > (size_t)cbBlock)
			return AllocT::allocate(cb);
		{
			Block* blk = m_freeList.pop();
			if (blk)
			{
				BOOST_MEMORY_ASSERT(AllocT::alloc_size(blk) >= cb);
				return blk;
			}
		}
		return AllocT::allocate(cbBlock);
	}

	void BOOST_MEMORY_CALL deallocate(void* p)
	{
#if defined(_DEBUG)
		((Block*)p)->prev(NULL);
#endif
		m_freeList.push((Block*)p);
	}

	static size_t BOOST_MEMORY_CALL alloc_size(void* p)
	{
		return AllocT::alloc_size(p);
	}

	void BOOST_MEMORY_CALL clear()
	{
		Block* freeList = m_freeList.clear();
		while (freeList)
		{
			Block* blk = freeList;
			freeList = freeList->prev();
			AllocT::deallocate(blk);
		}
	}
};

// -------------------------------------------------------------------------
// class system_pool_s

//template <class PolicyT, class StackT = default_stack>
template <class AllocT, class StackT = default_stack>

class system_pool_s
{
private:
	typedef system_pool_imp<AllocT, StackT> SystemPoolImpl;

	static SystemPoolImpl s_impl;

public:
	static void* BOOST_MEMORY_CALL allocate(size_t cb) { return s_impl.allocate(cb); }
	static void BOOST_MEMORY_CALL deallocate(void* p) { s_impl.deallocate(p); }
	static size_t BOOST_MEMORY_CALL alloc_size(void* p) {
		return s_impl.alloc_size(p);
	}
};

template <class PolicyT, class StackT>
system_pool_imp<PolicyT, StackT> system_pool_s<PolicyT, StackT>::s_impl;

// -------------------------------------------------------------------------

//typedef system_pool_s<NS_BOOST_MEMORY_POLICY::stdlib, default_stack> system_pool;

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_MEMORY_END

#endif /* __BOOST_MEMORY_SYSTEM_POOL_HPP__ */

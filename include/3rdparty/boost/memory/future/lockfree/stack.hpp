//
//  boost/lockfree/stack.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/lockfree/todo.htm for documentation.
//
#ifndef BOOST_LOCKFREE_STACK_HPP
#define BOOST_LOCKFREE_STACK_HPP

#ifndef BOOST_LOCKFREE_TAGGED_PTR_HPP
#include "tagged_ptr.hpp"
#endif

NS_BOOST_LOCKFREE_BEGIN

// -------------------------------------------------------------------------
// class stack

class stack
{
private:
	stack(const stack&);
	void operator=(const stack&);

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
	tagged_ptr<node> m_top;

	struct PushOp
	{
		node* m_val;
		PushOp(node* val) : m_val(val) {}
		node* BOOST_LOCKFREE_CALL operator()(node* top) const {
			m_val->_m_prev = top;
			return m_val;
		}
		static bool BOOST_LOCKFREE_CALL valid(node* top) {
			return true;
		}
	};

	struct PopOp
	{
		node* m_ret;
		PopOp() : m_ret(NULL) {}
		node* BOOST_LOCKFREE_CALL operator()(node* top) {
			m_ret = top;
			return top->_m_prev;
		}
		static bool BOOST_LOCKFREE_CALL valid(node* top) {
			return top != NULL;
		}
	};

public:
	stack() {}
	
	void BOOST_LOCKFREE_CALL push(node* val)
	{
		BOOST_DETAIL_ASSERT(val->_m_prev == NULL);
		PushOp pushOp(val);
		m_top.set(pushOp);
	}

	node* BOOST_LOCKFREE_CALL clear()
	{
		return m_top.clear();
	}

	node* BOOST_LOCKFREE_CALL pop()
	{
		PopOp popOp;
		m_top.set(popOp);
#if defined(_DEBUG)
		if (popOp.m_ret)
			popOp.m_ret->_m_prev = NULL;
#endif
		return popOp.m_ret;
	}
};

// -------------------------------------------------------------------------
// class typed_stack

template <class Type>
class typed_stack
{
private:
	stack m_impl;

public:
	void BOOST_LOCKFREE_CALL push(Type* val)
	{
		m_impl.push(val);
	}

	Type* BOOST_LOCKFREE_CALL clear()
	{
		return static_cast<Type*>(m_impl.clear());
	}

	Type* BOOST_LOCKFREE_CALL pop()
	{
		return static_cast<Type*>(m_impl.pop());
	}
};

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_LOCKFREE_END

#endif /* BOOST_LOCKFREE_STACK_HPP */

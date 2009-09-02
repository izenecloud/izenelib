/*
 * Copyright (c) 2006, Aleksey Cheusov <vle@gmx.net>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation.  I make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 */

#ifndef _JUDY_SET_CELL_H_
#define _JUDY_SET_CELL_H_

#include "judyarray/judy_common.h"

template <typename TKey>
class judy_set_cell {
private:
	Pvoid_t              m_judy;
	size_t               m_size;

	typedef judy_set_cell <TKey> __this_type;

// types
public:
	typedef TKey key_type;
	typedef TKey value_type;
	typedef TKey data_type;
	typedef TKey mapped_type;

	typedef size_t                    size_type;
	typedef ptrdiff_t                 difference_type;

	// I know about proxies, but I don't think they are really
	// necessary in the class implemeting a set.
	typedef const TKey *              pointer;
	typedef TKey *                    const_pointer;

	typedef TKey                      reference;
	typedef TKey                      const_reference;

// functions
public:
	judy_set_cell ()
	{
		m_judy = 0;
		m_size = 0;
	}

	template <class Tit>
	judy_set_cell (Tit beg, Tit end){
		m_judy = 0;
		m_size = 0;

		insert (beg, end);
	}

	judy_set_cell (const __this_type& a)
		:
		m_judy (0),
		m_size (0)
	{
		insert (a.begin (), a.end ());
	}

	~judy_set_cell ()
	{
		clear ();
		assert (m_judy == 0);
		assert (m_size == 0);
	}

	void clear ()
	{
#ifdef JUDYARRAY_DEBUG
		// Slow implementation which allows better
		// inconsistency checking
		erase (begin (), end ());
#else // JUDYARRAY_DEBUG
		// Much faster implementation

		judy1_freearray (m_judy);
		m_judy = 0;
		m_size = 0;
#endif // JUDYARRAY_DEBUG
	}

	judy_set_cell& operator = (const __this_type& a)
	{
		// exception-less implementation
		if (this != &a){
			clear ();

			__this_type temp (a);

			swap (temp);
		}

		return *this;
	}

	template <class Tit>
	void insert (Tit beg, Tit end)
	{
		while (beg != end){
			insert (*beg);

			++beg;
		}
	}

	bool empty () const
	{
		return m_judy == 0;
	}

	size_type bucket_count () const
	{
		return m_size;
	}

	void swap (__this_type& a)
	{
		std::swap (m_judy, a.m_judy);
		std::swap (m_size, a.m_size);
	}

	size_type size () const
	{
		return m_size;
	}

	size_type max_size () const
	{
		return size_type (-1);
	}

	class iterator {
	public:
		__JUDYARRAY_TYPEDEFS(__this_type);
		typedef std::forward_iterator_tag iterator_category;

		const __this_type *m_obj;

		Word_t                 m_index;
		bool                   m_end;

		iterator ()
			:
			m_obj   (NULL),
			m_index (0),
			m_end   (true)
		{
		}

		iterator (const __this_type *obj, Word_t index)
			:
			m_obj (obj),
			m_index (index),
			m_end (false)
		{
		}

		iterator (const __this_type *obj)
			:
			m_obj (obj),
			m_index (0),
			m_end (false)
		{
			if (!judy1_first (m_obj -> m_judy, &m_index))
				m_end = true;
		}

		iterator & operator = (const iterator& a)
		{
			if (this == &a)
				return *this;

			m_index       = a.m_index;
			m_obj         = a.m_obj;
			m_end         = a.m_end;

			return *this;
		}

		reference operator * ()
		{
			assert (!m_end);
			return (key_type) m_index;
		}
		const_reference operator * () const
		{
			assert (!m_end);
			return (key_type) m_index;
		}

		iterator& operator ++ ()
		{
			assert (!m_end);

			if (!judy1_next (m_obj -> m_judy, &m_index)){
				m_end = true;
			}

			return *this;
		}

		iterator operator ++ (int)
		{
			iterator ret = *this;
			operator ++ ();
			return ret;
		}

		bool operator == (const iterator& a) const
		{
			if (m_end && a.m_end)
				return true;

			assert (!m_obj || !a.m_obj || m_obj == a.m_obj);

			if (m_end != a.m_end)
				return false;

			return m_index == a.m_index;
		}
		bool operator != (const iterator& a) const
		{
			return !operator == (a);
		}
	};
	typedef iterator const_iterator;

	void erase (key_type key)
	{
		if (judy1_unset (m_judy, (Word_t) key))
			--m_size;
	}

	void erase (iterator f, iterator l)
	{
		while (f != l){
			erase (f++);
		}
	}

	void erase (iterator it)
	{
		if (!it.m_end){
			erase (*it);
		}else{
			// Do nothing for cont.erase (cont.end ())
		}
	}

	const_iterator find (key_type key) const
	{
		if (judy1_test (m_judy, (Word_t) key)){
			return iterator (this, (Word_t) key);
		}else{
			return iterator ();
		}
	}

	iterator find (key_type key)
	{
		if (judy1_test (m_judy, (Word_t) key)){
			return iterator (this, (Word_t) key);
		}else{
			return iterator ();
		}
	}

	size_type count (const key_type& key) const
	{
		if (judy1_test (m_judy, (Word_t) key))
			return 1;
		else
			return 0;
	}

	std::pair <iterator, bool> insert (key_type key)
	{
		if (judy1_set (m_judy, (Word_t) key)){
			++m_size;
			return std::make_pair (iterator (this, (Word_t) key), true);
		}else{
			return std::make_pair (iterator (this, (Word_t) key), false);
		}
	}

public:
	iterator begin ()
	{
		return iterator (this);
	}
	const_iterator begin () const
	{
		return const_iterator (this);
	}

	iterator end ()
	{
		return iterator ();
	}
	const_iterator end () const
	{
		return const_iterator ();
	}
};

#endif // _JUDY_SET_CELL_H_

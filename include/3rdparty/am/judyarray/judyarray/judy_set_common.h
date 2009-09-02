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

#ifndef _JUDY_SET_COMMON_H_
#define _JUDY_SET_COMMON_H_

template <typename TKey>
struct __judy_set_traits_base {
	typedef TKey                            key_type;
	typedef TKey                            data_type;
	typedef TKey                            mapped_type;
	typedef TKey                            value_type;
	typedef size_t                          size_type;
	typedef ptrdiff_t                       difference_type;

	// It is not possible to derive 'pointer' and 'reference' types from
	// TAllocator
	typedef value_type                   * pointer;
	typedef value_type             const * const_pointer;
	typedef value_type                   & reference;
	typedef value_type             const & const_reference;

	static inline const key_type &value2key (const value_type& value)
	{
		return value;
	}
	static inline const value_type &key2value (const key_type& key)
	{
		return key;
	}
};

#endif // _JUDY_SET_COMMON_H_

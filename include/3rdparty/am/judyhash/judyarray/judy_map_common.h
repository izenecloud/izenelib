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

#ifndef _JUDY_MAP_COMMON_H_
#define _JUDY_MAP_COMMON_H_

template <typename TKey, typename TData>
struct __judy_map_traits_base {
	typedef TKey                            key_type;
	typedef TData                           data_type;
	typedef TData                           mapped_type;
#ifdef JUDYARRAY_NO_CONST
	typedef std::pair <      TKey, TData>   value_type;
#else
	typedef std::pair <const TKey, TData>   value_type;
#endif

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
		return value.first;
	}

	static inline value_type key2value (const key_type& key)
	{
		return value_type (key, mapped_type ());
	}
};

#endif // _JUDY_MAP_COMMON_H_

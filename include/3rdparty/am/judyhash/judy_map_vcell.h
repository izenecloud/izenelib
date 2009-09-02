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

#ifndef _JUDY_MAP_H_
#define _JUDY_MAP_H_

#include "judyarray/judy_common.h"
#include "judyarray/judy_mapset_vcell_common.h"

//////////////////////////////////////////////////////////////////////

template <typename TKey, typename TData>
struct __judy_map_vcell_traits {
	typedef TKey                            key_type;
	typedef TData                           data_type;
	typedef TData                           mapped_type;
#ifdef JUDYARRAY_NO_CONST
	typedef std::pair <      TKey,       TData>  value_type;
#else
	typedef std::pair <const TKey, const TData>  value_type;
#endif

	typedef size_t                          size_type;
	typedef ptrdiff_t                       difference_type;

	typedef value_type                   * pointer;
	typedef value_type             const * const_pointer;
	typedef value_type                   & reference;
	typedef value_type             const & const_reference;

	static const key_type &value2key (const value_type& value)
	{
		return value.first;
	}
	static value_type key2value (const key_type& key)
	{
		return value_type (key, mapped_type ());
	}
};

//////////////////////////////////////////////////////////////////////
/////////////////////// JUDY_MAP_VCELL ///////////////////////////////

template <
	typename TKey,
	typename TData,
	typename THashFunc, // = std::hash <Key>,
	typename TEqualFunc = std::equal_to <TKey>,
	typename TAllocator = std::allocator < typename __judy_map_vcell_traits <TKey, TData>::value_type > >
class judy_map_vcell
{
private:
	typedef __judy_mapset_base <
		TKey, TData, THashFunc, TEqualFunc, TAllocator,
		__judy_map_vcell_traits <TKey, TData> > __impl;
	typedef judy_map_vcell <
		TKey, TData, THashFunc, TEqualFunc, TAllocator
		> __this_type;

	__impl m_hash_base;

public:

	__JUDYARRAY_TYPEDEFS(__impl)

// rempping judy_map/judy_set common members
	REMAP_FUNCALLS(__impl, judy_map_l, m_hash_base)
/*
// judy_mem unique members
	mapped_type& operator [] (const key_type& key)
	{
		std::pair <iterator, bool> res = insert (
			value_type (key, mapped_type ()));

		return (mapped_type&) res.first -> second;
	}
*/
};

#endif // _JUDY_MAP_H_

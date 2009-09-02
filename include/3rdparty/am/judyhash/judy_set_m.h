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

#ifndef _JUDY_SET_M_H_
#define _JUDY_SET_M_H_

#include "judyarray/judy_common.h"
#include "judyarray/judy_mapset_map.h"
#include "judyarray/judy_mapset_common.h"
#include "judyarray/judy_set_common.h"

template <typename TKey, typename TLessFunc>
class __judy_set_traits_map
:
public __judy_cmp_traits_map <TKey, void, TLessFunc, __judy_set_traits_base <TKey> >
{
};

template <
	typename TKey,
	typename THashFunc, // = std::hash <Key>,
	typename TLessFunc = std::less <TKey>,
	typename TEqualFunc = std::equal_to <TKey>,
	typename TAllocator = std::allocator < TKey> >
class judy_set_m
{
private:
	typedef __judy_mapset_base <
		TKey, char, THashFunc, TEqualFunc, TAllocator,
		__judy_set_traits_map <TKey, TLessFunc> > __impl;
	typedef judy_set_m <
		TKey, THashFunc, TLessFunc, TEqualFunc, TAllocator> __this_type;

	__impl m_hash_base;

public:

	__JUDYARRAY_TYPEDEFS(__impl)

// rempping judy_map/judy_set common members
	REMAP_FUNCALLS(__impl, judy_set_m, m_hash_base)

// judy_mem unique members
};

#endif // _JUDY_SET_M_H_

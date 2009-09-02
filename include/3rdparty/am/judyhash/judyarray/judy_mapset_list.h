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

#ifndef _JUDY_MAPSET_LIST_H_
#define _JUDY_MAPSET_LIST_H_

#include <list>

template <typename TKey, typename TData,
	typename TEqualFunc, typename TTraits>
class __judy_cmp_traits_list
:
public TTraits
{
public:
	class __pointers_list_type
	:
	public std::list <typename TTraits::pointer>
	{
	private:
		TEqualFunc m_eq_func;
		typedef std::list <typename TTraits::pointer> inherited;

	public:
		typedef typename inherited::pointer pointer;
		typedef typename inherited::value_type value_type;
		typedef typename inherited::iterator iterator;

		__pointers_list_type ()
		{
		}
		~__pointers_list_type ()
		{
		}

		iterator find (const TKey &key)
		{
			iterator b = this -> begin ();
			iterator e = this -> end ();

			for (; !(b == e); ++b){
				if (m_eq_func (TTraits::value2key (**b), key)){
					return b;
				}
			}

			return e;
		}

		std::pair <iterator, bool> insert (const value_type& v)
		{
			const iterator e = this -> end ();
			const iterator f = this -> find (TTraits::value2key(*v));

			if (e == f){
				return std::make_pair (
					inherited::insert (inherited::end (), v), true);
			}else{
				return std::make_pair (f, false);
			}
		}
	};

	typedef __pointers_list_type pointers_list_type;
};

#endif // _JUDY_MAPSET_LIST_H_

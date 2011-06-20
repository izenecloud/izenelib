//
// mp::sparse_array
//
// Copyright (C) 2008-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MP_SPARSE_ARRAY_H__
#define MP_SPARSE_ARRAY_H__

#include <vector>
#include <cstdlib>

namespace mp {


template <typename T>
class sparse_array {
public:
	typedef size_t size_type;

	sparse_array();
	~sparse_array();

	//! Set instance of T into index.
	inline T& set(size_type index);
	template <typename A1>
	inline T& set(size_type index, A1 a1);
	template <typename A1, typename A2>
	inline T& set(size_type index, A1 a1, A2 a2);
	template <typename A1, typename A2, typename A3>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3);
	template <typename A1, typename A2, typename A3, typename A4>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4);
	template <typename A1, typename A2, typename A3, typename A4, typename A5>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);
	template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	inline T& set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);

	//! Reset index.
	inline void reset(size_type index);

	//! Get the data of index.
	inline T& data(size_type index);
	inline const T& data(size_type index) const;

	//! Get the data of index.
	inline T* get(size_type index);
	inline const T* get(size_type index) const;

	//! Return true if index is set otherwise false.
	inline bool test(size_type index) const;

	inline size_type capacity() const;

private:
	static const size_t EXTEND_ARRAY_SIZE = 64;

	struct item_t {
		bool enable;
		char data[sizeof(T)];
	};

	typedef std::vector<item_t*> base_array_t;
	base_array_t base_array;

private:
	inline void* set_impl(size_type index);
	inline void revert(size_type index);
	inline void extend_array();
	inline item_t& item_of(size_type index);
	inline const item_t& item_of(size_type index) const;
};


template <typename T>
sparse_array<T>::sparse_array()
{
	extend_array();
}

template <typename T>
sparse_array<T>::~sparse_array()
{
	for(typename base_array_t::iterator it(base_array.begin()), it_end(base_array.end());
			it != it_end;
			++it ) {
		for(item_t *p(*it), *p_end(p+EXTEND_ARRAY_SIZE); p != p_end; ++p) {
			if(p->enable) {
				reinterpret_cast<T*>(p->data)->~T();
			}
		}
		std::free(*it);
	}
}

template <typename T>
T& sparse_array<T>::set(size_type index)
try {
	return *(new (set_impl(index)) T());
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1>
T& sparse_array<T>::set(size_type index, A1 a1)
try {
	return *(new (set_impl(index)) T(a1));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2)
try {
	return *(new (set_impl(index)) T(a1, a2));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3)
try {
	return *(new (set_impl(index)) T(a1, a2, a3));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15));
} catch (...) {
	revert(index);
	throw;
}
template <typename T>
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
T& sparse_array<T>::set(size_type index, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
try {
	return *(new (set_impl(index)) T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16));
} catch (...) {
	revert(index);
	throw;
}

template <typename T>
void sparse_array<T>::reset(size_type index)
{
	item_t& item(item_of(index));
	item.enable = false;
	reinterpret_cast<T*>(item.data)->~T();
}

template <typename T>
T& sparse_array<T>::data(size_type index)
{
	return *reinterpret_cast<T*>(item_of(index).data);
}

template <typename T>
const T& sparse_array<T>::data(size_type index) const
{
	return *reinterpret_cast<const T*>(item_of(index).data);
}

template <typename T>
T* sparse_array<T>::get(size_type index)
{
	if( base_array.size() * EXTEND_ARRAY_SIZE > index ) {
		item_t& item(item_of(index));
		if( item.enable ) {
			return reinterpret_cast<T*>(item.data);
		}
	}
	return NULL;
}

template <typename T>
const T* sparse_array<T>::get(size_type index) const
{
	if( base_array.size() * EXTEND_ARRAY_SIZE > index ) {
		item_t& item(item_of(index));
		if( item.enable ) {
			return reinterpret_cast<const T*>(item.data);
		}
	}
	return NULL;
}

template <typename T>
bool sparse_array<T>::test(size_type index) const
{
	return base_array.size() * EXTEND_ARRAY_SIZE > index &&
		item_of(index).enable;
}

template <typename T>
typename sparse_array<T>::size_type sparse_array<T>::capacity() const
{
	return base_array.size() * EXTEND_ARRAY_SIZE;
}

template <typename T>
void* sparse_array<T>::set_impl(size_type index)
{
	while( base_array.size() <= index / EXTEND_ARRAY_SIZE ) {
		extend_array();
	}
	item_t& item(item_of(index));
	if( item.enable ) {
		reinterpret_cast<T*>(item.data)->~T();
	} else {
		item.enable = true;
	}
	return item.data;
}

template <typename T>
void sparse_array<T>::revert(size_type index)
{
	item_of(index).enable = false;
}

template <typename T>
void sparse_array<T>::extend_array()
{
	item_t* ex = (item_t*)std::calloc(EXTEND_ARRAY_SIZE, sizeof(item_t));
	if(!ex) { throw std::bad_alloc(); }
	base_array.push_back(ex);
}

template <typename T>
typename sparse_array<T>::item_t& sparse_array<T>::item_of(size_type index)
{
	return base_array[index / EXTEND_ARRAY_SIZE][index % EXTEND_ARRAY_SIZE];
}

template <typename T>
const typename sparse_array<T>::item_t& sparse_array<T>::item_of(size_type index) const
{
	return base_array[index / EXTEND_ARRAY_SIZE][index % EXTEND_ARRAY_SIZE];
}


}  // namespace mp

#endif /* mp/sparse_array.h */


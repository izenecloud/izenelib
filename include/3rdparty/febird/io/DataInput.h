/* vim: set tabstop=4 : */
#ifndef __febird_io_DataInput_h__
#define __febird_io_DataInput_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <string.h>
#include <typeinfo>
#include <stdexcept>
#include <string>
#include <sstream>

#include <deque>
#include <list>
#include <map>
#include <set>
#include <vector>

#include <boost/version.hpp>

#if BOOST_VERSION < 103301
# include <boost/limits.hpp>
# include <boost/detail/limits.hpp>
#else
# include <boost/detail/endian.hpp>
#endif
#include <boost/cstdint.hpp>
#include <boost/type_traits.hpp>
#include <boost/ref.hpp>

#if !defined(BOOST_BIG_ENDIAN) && !defined(BOOST_LITTLE_ENDIAN)
	#error must define byte endian
#endif

#include "../pass_by_value.h"
#include "byte_swap.h"
#include "DataIO_Basic.h"
#include "DataIO_Version.h"
#include "DataIO_Tuple.h"
#include "DataIO_Exception.h"
#include "var_int.h"

#if !defined(BOOST_BIG_ENDIAN) && !defined(BOOST_LITTLE_ENDIAN)
# error must define byte endian
#endif

namespace febird {

template<class Input, class Class>
void DataIO_loadObject(Input& input, Class& x)
{
#if 1//DATA_IO_ALLOW_DEFAULT_SERIALIZE
	input.ensureRead(&x, sizeof(Class));
#else
	x.MustDefineCustomLoad(input);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template<class DataIO, class T, class Alloc>
void DataIO_load_vector_raw(DataIO& dio, std::vector<T, Alloc>& x, boost::mpl::false_ bswap)
{
	dio.ensureRead(&*x.begin(), sizeof(T) * x.size());
}

template<class DataIO, class T, class Alloc>
void DataIO_load_vector_raw(DataIO& dio, std::vector<T, Alloc>& x, boost::mpl::true_ bswap)
{
	dio.ensureRead(&*x.begin(), sizeof(T) * x.size());
	byte_swap(&*x.begin(), x.size());
}

template<class DataIO, class T, class Alloc, class Bswap>
void DataIO_load_vector_aux(DataIO& dio, std::vector<T, Alloc>& x,
							Bswap bswap, ::boost::mpl::true_ real_dumpable)
{
	DataIO_load_vector_raw(dio, x, bswap);
}

template<class DataIO, class T, class Alloc, class Bswap>
void DataIO_load_vector_aux(DataIO& dio, std::vector<T, Alloc>& x,
							Bswap bswap, ::boost::mpl::false_ real_dumpable)
{
	size_t i = x.size();
	T* first = &*x.begin();
	try {
		for (; i; --i, ++first)
			dio >> *first;
	}
	catch (...)
	{
		x.resize(x.size() - i);
		throw;
	}
}

template<class DataIO, class T, class Alloc, int Size, bool Dumpable, class Bswap>
void DataIO_load_vector_opt(DataIO& dio, std::vector<T, Alloc>& x, Bswap bswap,
							 DataIO_is_realdump<DataIO, T, Size, Dumpable> tag)
{
	// sizeof(T) == Size implies T has no paddings
	BOOST_STATIC_ASSERT(sizeof(T) >= Size);
	typedef ::boost::mpl::bool_<sizeof(T)==Size && Dumpable> is_realdump_t;
	DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, is_realdump_t);
	var_uint32_t size0; dio >> size0;
	x.resize(size0.t);
	if (size0.t)
		DataIO_load_vector_aux(dio, x, bswap, is_realdump_t());
}

template<class DataIO, class T1, class T2, class Alloc, class Bswap>
void DataIO_load_vector(DataIO& dio, std::vector<std::pair<T1,T2>, Alloc>& x, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		std::pair<T1,T2>,
		sizeof(T1)+sizeof(T2),
		DataIO_is_dump<DataIO, std::pair<T1,T2> >::value
	> tag(NULL);
	DataIO_load_vector_opt(dio, x, bs, tag);
}

template<class DataIO, class T, class Alloc, class Bswap>
void DataIO_load_vector(DataIO& dio, std::vector<T, Alloc>& x, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		T,
		sizeof(T),
		DataIO_is_dump<DataIO, T>::value
	> tag(NULL);
	DataIO_load_vector_opt(dio, x, bs, tag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<class DataIO, class T>
void DataIO_load_array_raw(DataIO& dio, T* a, size_t n, boost::mpl::false_ bswap)
{
	dio.ensureRead(a, sizeof(T) * n);
}

template<class DataIO, class T>
void DataIO_load_array_raw(DataIO& dio, T* a, size_t n, boost::mpl::true_ bswap)
{
	dio.ensureRead(a, sizeof(T) * n);
	byte_swap(a, n);
}

template<class DataIO, class T, class Bswap>
void DataIO_load_array_aux(DataIO& dio, T* a, size_t n, Bswap bswap,
					   ::boost::mpl::true_ real_dumpable)
{
	DataIO_load_array_raw(dio, a, n, bswap);
}

template<class DataIO, class T, class Bswap>
void DataIO_load_array_aux(DataIO& dio, T* a, size_t n, Bswap bswap,
					   ::boost::mpl::false_ real_dumpable)
{
	for (size_t i = n; i; --i, ++a)
		dio >> *a;
}

template<class DataIO, class T, int Size, bool Dumpable, class Bswap>
void DataIO_load_array_opt(DataIO& dio, T* a, size_t n, Bswap bswap,
							 DataIO_is_realdump<DataIO, T, Size, Dumpable> tag)
{
	// sizeof(T) == Size implies T has no paddings
	BOOST_STATIC_ASSERT(sizeof(T) >= Size);
	typedef ::boost::mpl::bool_<sizeof(T)==Size && Dumpable> is_realdump_t;
	DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, is_realdump_t);
	DataIO_load_array_aux(dio, a, n, bswap, is_realdump_t());
}

template<class DataIO, class T1, class T2, class Bswap>
void DataIO_load_array(DataIO& dio, std::pair<T1,T2>* a, size_t n, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		std::pair<T1,T2>,
		sizeof(T1)+sizeof(T2),
		DataIO_is_dump<DataIO, std::pair<T1,T2> >::value
	> tag(NULL);
	DataIO_load_array_opt(dio, a, n, bs, tag);
}

template<class DataIO, class T, class Bswap>
void DataIO_load_array(DataIO& dio, T* a, size_t n, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		T,
		sizeof(T),
		DataIO_is_dump<DataIO, T>::value
	> tag(NULL);
	DataIO_load_array_opt(dio, a, n, bs, tag);
}

//////////////////////////////////////////////////////////////////////////
template<class DataIO, class T>
void DataIO_load_elem_raw(DataIO& dio, T& x, boost::mpl::false_ bswap)
{
	dio.ensureRead(&x, sizeof(T));
}

template<class DataIO, class T>
void DataIO_load_elem_raw(DataIO& dio, T& x, boost::mpl::true_ bswap)
{
	dio.ensureRead(&x, sizeof(T));
	byte_swap_in(x, bswap);
}

template<class DataIO, class T, class Bswap>
void DataIO_load_elem_aux(DataIO& dio, T& x, Bswap bswap, boost::mpl::true_ real_dumpable)
{
	DataIO_load_elem_raw(dio, x, bswap);
}

template<class DataIO, class T, class Bswap>
void DataIO_load_elem_aux(DataIO& dio, T& x, Bswap bswap, boost::mpl::false_ real_dumpable)
{
	DataIO_loadObject(dio, x);
}

template<class DataIO, class T, int Size, bool Dumpable, class Bswap>
void DataIO_load_elem_opt(DataIO& dio, T& x, Bswap bswap,
					  DataIO_is_realdump<DataIO, T, Size, Dumpable> tag)
{
	BOOST_STATIC_ASSERT(sizeof(T) >= Size);
	typedef ::boost::mpl::bool_<sizeof(T)==Size && Dumpable> is_realdump_t;
	DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, is_realdump_t);
	DataIO_load_elem_aux(dio, x, bswap, is_realdump_t());
}

template<class DataIO, class T, class Bswap>
void DataIO_load_elem(DataIO& dio, T& x, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		T,
		sizeof(T),
		DataIO_is_dump<DataIO, T>::value
	> tag(NULL);
	DataIO_load_elem_opt(dio, x, bs, tag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
template<class BaseInput, class Final_Input>
class DataInput : public BaseInput
{
public:
	typedef typename BaseInput::is_seekable  is_seekable;
	typedef typename BaseInput::stream_t     stream_t;
	typedef boost::mpl::true_  is_loading;
	typedef boost::mpl::false_ is_saving;

	using BaseInput::operator>>;

	//////////////////////////////////////////////////////////////////////////
	//! dual operator, for auto dispatch single serialize proc...
	//!
	//! for PrimitiveInputImpl, operator& is input,
	//! for DataOutput, operator& is output.
	template<class T> Final_Input& operator&(T& x) { return operator>>(x); }

	//-------------------------------------------------------------------------------------------------
	//! can not bound temp object to non-const reference,
	//! so use pass_by_value object in this case
	//! @{
	template<class T> Final_Input& operator& (pass_by_value<T> x) { return (*this) >> x.val; }
	template<class T> Final_Input& operator>>(pass_by_value<T> x) { return (*this) >> x.val; }
	//@}
	//-------------------------------------------------------------------------------------------------

	template<class T> Final_Input& operator& (boost::reference_wrapper<T> x) { return (*this) >> x.get(); }
	template<class T> Final_Input& operator>>(boost::reference_wrapper<T> x) { return (*this) >> x.get(); }

	template<int Dim> Final_Input& operator>>(char (&x)[Dim]) { return this->load(x, Dim); }
	template<int Dim> Final_Input& operator>>(unsigned char (&x)[Dim]) { return this->load(x, Dim); }
	template<int Dim> Final_Input& operator>>(  signed char (&x)[Dim]) { return this->load(x, Dim); }

#ifdef DATA_IO_SUPPORT_SERIALIZE_PTR
	template<class T> Final_Input& operator>>(T*& x)
	{
		x = new T;
		*this >> *x;
		return static_cast<Final_Input&>(*this);
	}
#else
	template<class T> Final_Input& operator>>(T*& x)
	{
		T::NotSupportSerializePointer();
		return static_cast<Final_Input&>(*this);
	}
#endif

	//!@{
	//! standard container this->....
	//--------------------------------------------------------------------
	struct Container_insert
	{
		template<class Container, class Element>
		void operator()(Container& cont, const Element& e) const
		{ cont.insert(e); }
	};
	struct Container_push_back
	{
		template<class Container, class Element>
		void operator()(Container& cont, const Element& e) const
		{ cont.push_back(e); }
	};

	template<class ContainerType, class InsertOp>
	Final_Input& container_load(ContainerType& x, InsertOp insert)
	{
		x.clear();
		var_uint32_t size;
		*this >> size;
		for (uint32_t i = 0; i < size.t; ++i)
		{
			typename ContainerType::value_type e;
			*this >> e;
			insert(x, e);
		}
		return static_cast<Final_Input&>(*this);
	}
	//--------------------------------------------------------------------

	template<class First, class Second>
	Final_Input& operator>>(std::pair<First, Second>& x)
	{
		return *this >> x.first >> x.second;
	}
	template<class KeyT, class ValueT, class Compare, class Alloc>
	Final_Input& operator>>(std::map<KeyT, ValueT, Compare, Alloc>& x)
	{
		var_uint32_t size; *this >> size;
		x.clear();
		for (uint32_t i = 0; i < size.t; ++i)
		{
			std::pair<KeyT, ValueT> e;
			*this >> e;
			x.insert(e);
		}
		return static_cast<Final_Input&>(*this);
	}
	template<class KeyT, class ValueT, class Compare, class Alloc>
	Final_Input& operator>>(std::multimap<KeyT, ValueT, Compare, Alloc>& x)
	{
		var_uint32_t size; *this >> size;
		x.clear();
		for (uint32_t i = 0; i < size.t; ++i)
		{
			std::pair<KeyT, ValueT> e;
			*this >> e;
			x.insert(e);
		}
		return static_cast<Final_Input&>(*this);
	}
	template<class ValueT, class Compare, class Alloc>
	Final_Input& operator>>(std::set<ValueT, Compare, Alloc>& x)
	{
		return container_load(x, Container_insert());
	}
	template<class ValueT, class Compare, class Alloc>
	Final_Input& operator>>(std::multiset<ValueT, Compare, Alloc>& x)
	{
		return container_load(x, Container_insert());
	}
	template<class ValueT, class Alloc>
	Final_Input& operator>>(std::list<ValueT, Alloc>& x)
	{
		return container_load(x, Container_push_back());
	}
	template<class ValueT, class Alloc>
	Final_Input& operator>>(std::deque<ValueT, Alloc>& x)
	{
		return container_load(x, Container_push_back());
	}
	//!@}
};

//////////////////////////////////////////////////////////////////////////

template<class BaseInput, class Final_Input>
class VarIntVarInput : public BaseInput
{
protected:
	uint32_t load_var_uint32()
	{
		uint32_t v = 0;
		for (int shift = 0; shift < 35; shift += 7)
		{
			byte b; *this >> b;
			v |= uint32_t(b & 0x7F) << shift;
			if ((b & 0x80) == 0)
				return v;
		}
		assert(0); // should not get here
		return v; // avoid compile warning
	}
	uint64_t load_var_uint64()
	{
		uint64_t v = 0;
		byte b;
		for (int shift = 0; shift < 56; shift += 7)
		{
			*this >> b;
			v |= uint64_t(b & 0x7F) << shift;
			if ((b & 0x80) == 0)
				return v;
		}
		*this >> b;
		v |= uint64_t(b) << 56;
		return v;
	}
public:
	using BaseInput::operator>>;
	Final_Input& operator>>(var_int16_t& x)
	{
		x.t = var_int16_u2s(load_var_uint32());
		return static_cast<Final_Input&>(*this);
	}
	Final_Input& operator>>(var_int32_t& x)
	{
		x.t = var_int32_u2s(load_var_uint32());
		return static_cast<Final_Input&>(*this);
	}
	Final_Input& operator>>(var_uint16_t& x) { x.t = load_var_uint32(); return static_cast<Final_Input&>(*this); }
	Final_Input& operator>>(var_uint32_t& x) { x.t = load_var_uint32(); return static_cast<Final_Input&>(*this); }

#if !defined(BOOST_NO_INT64_T)
	Final_Input& operator>>(var_int64_t& x)
	{
		x.t = var_int64_u2s(load_var_uint64());
		return static_cast<Final_Input&>(*this);
	}
	Final_Input& operator>>(var_uint64_t& x) { x.t = load_var_uint64(); return static_cast<Final_Input&>(*this); }
#endif
	Final_Input& operator>>(serialize_version_t& x) { x.t = load_var_uint32(); return static_cast<Final_Input&>(*this); }
};

template<class BaseInput, class Final_Input>
class VarIntFixedInput : public BaseInput
{
public:
	using BaseInput::operator>>;
	Final_Input& operator>>(var_int16_t & x) { return *this >> x.t; }
	Final_Input& operator>>(var_uint16_t& x) { return *this >> x.t; }

	Final_Input& operator>>(var_int32_t & x) { return *this >> x.t; }
	Final_Input& operator>>(var_uint32_t& x) { return *this >> x.t; }

#if !defined(BOOST_NO_INT64_T)
	Final_Input& operator>>(var_int64_t & x) { return *this >> x.t; }
	Final_Input& operator>>(var_uint64_t& x) { return *this >> x.t; }
#endif
	Final_Input& operator>>(serialize_version_t& x) { return *this >> x.t; }
};

template<class BaseInput, class Final_Input>
class BigEndianStringInput : public BaseInput
{
public:
	using BaseInput::operator>>;

	Final_Input& load(wchar_t* s, size_t n)
	{
		this->ensureRead(s, sizeof(wchar_t)*n);
#ifdef BOOST_LITTLE_ENDIAN
		byte_swap(s, n);
#endif
		return static_cast<Final_Input&>(*this);
	}
#ifndef BOOST_NO_INTRINSIC_WCHAR_T
	Final_Input& operator>>(wchar_t& x)
	{
		this->ensureRead(&x, sizeof(x));
#ifdef BOOST_LITTLE_ENDIAN
		x = byte_swap(x);
#endif
		return static_cast<Final_Input&>(*this); 
	}
#endif
};

template<class BaseInput, class Final_Input>
class LittleEndianStringInput : public BaseInput
{
public:
	using BaseInput::operator>>;

	Final_Input& load(wchar_t* s, size_t n)
	{
		this->ensureRead(s, sizeof(wchar_t)*n);
#ifdef BOOST_BIG_ENDIAN
		byte_swap(s, n);
#endif
		return static_cast<Final_Input&>(*this); 
	}
#ifndef BOOST_NO_INTRINSIC_WCHAR_T
	Final_Input& operator>>(wchar_t& x)
	{
		this->ensureRead(&x, sizeof(x));
#ifdef BOOST_BIG_ENDIAN
		x = byte_swap(x);
#endif
		return static_cast<Final_Input&>(*this); 
	}
#endif
};

template<class BaseInput, class Final_Input>
class CommonStringInput : public BaseInput
{
public:
	using BaseInput::load;
	using BaseInput::operator>>;

	Final_Input& load(char* s, size_t n) { this->ensureRead(s, n); return static_cast<Final_Input&>(*this); }
	Final_Input& load(unsigned char* s, size_t n) { this->ensureRead(s, n); return static_cast<Final_Input&>(*this); }
	Final_Input& load(  signed char* s, size_t n) { this->ensureRead(s, n); return static_cast<Final_Input&>(*this); }

	Final_Input& operator>>(char*& s) { return load_s0(s); }
	Final_Input& operator>>(wchar_t*& s) { return load_s0(s); }

	Final_Input& operator>>(std:: string& x) { return load_s1(x); }
	Final_Input& operator>>(std::wstring& x) { return load_s1(x); }

private:
	template<class ChT> Final_Input& load_s0(ChT*& s)
	{
		assert(0 == s);
		var_uint32_t n;
		*this >> n;
		s = new ChT[n.t+1];
		this->load(s, n.t);
		s[n] = 0;
		return static_cast<Final_Input&>(*this);
	}

	//! string in file format: [length : ....content.... ]
	template<class CharType, class Traits, class Allocator>
	Final_Input& load_s1(std::basic_string<CharType, Traits, Allocator>& x)
	{
		var_uint32_t length;
		*this >> length;
		x.resize(length.t); // str will be allocated at least (length+1) chars..
		if (length.t)
		{
			//	CharType* data = const_cast<CharType*>(str.data());
			CharType* data = &*x.begin(); // this will make a mutable string content
			this->load(data, length.t);
			//	data[length.t] = 0; // in most string implementation, this is accessible
			//	data[length.t] = 0; // in some string implementation, this is out of string bound
		}
		return static_cast<Final_Input&>(*this);
	}
};

template<class StreamT, class Final_Input>
class PrimitiveInputImpl : public StreamT
{
	//  this will cause compile error!!
	//	DECLARE_NONE_COPYABLE_CLASS(PrimitiveInputImpl)
public:
	typedef StreamT stream_t;

	StreamT* getStream() { return this; }

	Final_Input& operator>>(char& x) { x = (char)this->readByte(); return static_cast<Final_Input&>(*this); }
	Final_Input& operator>>(unsigned char& x) { x = (unsigned char)this->readByte(); return static_cast<Final_Input&>(*this); }
	Final_Input& operator>>(  signed char& x) { x = (  signed char)this->readByte(); return static_cast<Final_Input&>(*this); }
};

template<class StreamT, class Final_Input>
class PrimitiveInputImpl<StreamT*, Final_Input>
{
protected:
	StreamT* stream;

public:
	typedef StreamT stream_t;
	typedef typename stream_t::is_seekable is_seekable;

	void attach(stream_t* stream) { this->stream = stream; }

	StreamT* getStream() { return stream; }

	//! only delegate this method of StreamT
	size_t read(void* data, size_t length) { return stream->read(data, length); }

	void ensureRead(void* data, size_t length) { stream->ensureRead(data, length); }

	Final_Input& operator>>(char& x) { x = (char)stream->readByte(); return static_cast<Final_Input&>(*this); }
	Final_Input& operator>>(byte& x) { x = (byte)stream->readByte(); return static_cast<Final_Input&>(*this); }
	Final_Input& operator>>(signed char& x) { x = (signed char)stream->readByte(); return static_cast<Final_Input&>(*this); }
};

#define DATA_IO_GEN_DUMP_INPUT(Type)		\
	Final_Input& operator>>(Type& x)		\
	{	this->ensureRead(&x, sizeof(Type));	\
		return static_cast<Final_Input&>(*this);\
	}										\
	template<int Dim>						\
	Final_Input& operator>>(Type (&x)[Dim])	\
	{	this->ensureRead(x, sizeof(Type)*Dim);\
		return static_cast<Final_Input&>(*this); \
	}										\
	template<class Alloc>					\
	Final_Input& operator>>(std::vector<Type, Alloc>& x)\
	{	var_uint32_t n;						\
		static_cast<Final_Input&>(*this) >> n;\
		x.resize(n.t);						\
	    if (n.t)							\
		  this->ensureRead(&*x.begin(), sizeof(Type)*x.size());\
		return static_cast<Final_Input&>(*this); \
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_GEN_BSWAP_INT_INPUT(Int)	\
	Final_Input& operator>>(Int& x) {		\
	  this->ensureRead(&x, sizeof(Int));	\
	  x = byte_swap(x);						\
	  return static_cast<Final_Input&>(*this); \
	}										\
	template<int Dim>						\
	Final_Input& operator>>(Int (&x)[Dim]) {\
	  this->ensureRead(x, sizeof(Int)*Dim); \
	  byte_swap(x, Dim); \
	  return static_cast<Final_Input&>(*this); \
	}										\
	template<class Alloc>					\
	Final_Input& operator>>(std::vector<Int, Alloc>& x) {\
	  var_uint32_t n;						\
	  static_cast<Final_Input&>(*this) >> n;\
	  x.resize(n.t);						\
	  if (n.t) {							\
	    this->ensureRead(&*x.begin(), sizeof(Int)*x.size());\
	    byte_swap(x.begin(), x.end());		\
	  }										\
	  return static_cast<Final_Input&>(*this); \
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<class BaseInput, class Final_Input>
class BinFloatInput : public BaseInput
{
public:
	using BaseInput::operator>>;
	DATA_IO_GEN_DUMP_INPUT(float)
	DATA_IO_GEN_DUMP_INPUT(double)
	DATA_IO_GEN_DUMP_INPUT(long double)
};

template<class StreamT, class Final_Input>
class PortableIntegerInput : public PrimitiveInputImpl<StreamT, Final_Input>
{
public:

#ifdef BOOST_LITTLE_ENDIAN
	#define DATA_IO_GEN_PORTABLE_INT_INPUT  DATA_IO_GEN_BSWAP_INT_INPUT
#elif defined(BOOST_BIG_ENDIAN)
	#define DATA_IO_GEN_PORTABLE_INT_INPUT  DATA_IO_GEN_DUMP_INPUT
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif

	using PrimitiveInputImpl<StreamT, Final_Input>::operator>>;

	DATA_IO_GEN_PORTABLE_INT_INPUT(short)
	DATA_IO_GEN_PORTABLE_INT_INPUT(unsigned short)
	DATA_IO_GEN_PORTABLE_INT_INPUT(int)
	DATA_IO_GEN_PORTABLE_INT_INPUT(unsigned int)
	DATA_IO_GEN_PORTABLE_INT_INPUT(long)
	DATA_IO_GEN_PORTABLE_INT_INPUT(unsigned long)

#if defined(BOOST_HAS_LONG_LONG)
	DATA_IO_GEN_PORTABLE_INT_INPUT(long long)
	DATA_IO_GEN_PORTABLE_INT_INPUT(unsigned long long)
#elif defined(BOOST_HAS_MS_INT64)
	DATA_IO_GEN_PORTABLE_INT_INPUT(__int64)
	DATA_IO_GEN_PORTABLE_INT_INPUT(unsigned __int64)
#endif

	template<class T> Final_Input& operator>>(T& x)
	{
		DataIO_load_elem(static_cast<Final_Input&>(*this), x, DATA_IO_BSWAP_FOR_BIG(T)());
		return static_cast<Final_Input&>(*this);
	}

	template<class T, int Dim>
	Final_Input& operator>>(T (&x)[Dim])
	{ 
		DataIO_load_array(static_cast<Final_Input&>(*this), x, Dim, DATA_IO_BSWAP_FOR_BIG(T)());
		return static_cast<Final_Input&>(*this);
	}

	template<class T, class Alloc>
	Final_Input& operator>>(std::vector<T, Alloc>& x)
	{
		DataIO_load_vector(static_cast<Final_Input&>(*this), x, DATA_IO_BSWAP_FOR_BIG(T)());
		return static_cast<Final_Input&>(*this);
	}
};

template<class StreamT, class Final_Input>
class LittleEndianIntegerInput : public PrimitiveInputImpl<StreamT, Final_Input>
{
public:

#ifdef BOOST_LITTLE_ENDIAN
	#define DATA_IO_GEN_LITTLE_INT_INPUT  DATA_IO_GEN_DUMP_INPUT
#elif defined(BOOST_BIG_ENDIAN)
	#define DATA_IO_GEN_LITTLE_INT_INPUT  DATA_IO_GEN_BSWAP_INT_INPUT
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif

	typedef boost::mpl::false_ need_byte_swap;

	using PrimitiveInputImpl<StreamT, Final_Input>::operator>>;

	DATA_IO_GEN_LITTLE_INT_INPUT(short)
	DATA_IO_GEN_LITTLE_INT_INPUT(unsigned short)
	DATA_IO_GEN_LITTLE_INT_INPUT(int)
	DATA_IO_GEN_LITTLE_INT_INPUT(unsigned int)
	DATA_IO_GEN_LITTLE_INT_INPUT(long)
	DATA_IO_GEN_LITTLE_INT_INPUT(unsigned long)

#if defined(BOOST_HAS_LONG_LONG)
	DATA_IO_GEN_LITTLE_INT_INPUT(long long)
	DATA_IO_GEN_LITTLE_INT_INPUT(unsigned long long)
#elif defined(BOOST_HAS_MS_INT64)
	DATA_IO_GEN_LITTLE_INT_INPUT(__int64)
	DATA_IO_GEN_LITTLE_INT_INPUT(unsigned __int64)
#endif

	template<class T> Final_Input& operator>>(T& x)
	{
		DataIO_load_elem(static_cast<Final_Input&>(*this), x, DATA_IO_BSWAP_FOR_LITTLE(T)());
		return static_cast<Final_Input&>(*this);
	}

	template<class T, int Dim>
	Final_Input& operator>>(T (&x)[Dim])
	{ 
		DataIO_load_array(static_cast<Final_Input&>(*this), x, Dim, DATA_IO_BSWAP_FOR_LITTLE(T)());
		return static_cast<Final_Input&>(*this);
	}

	template<class T, class Alloc>
	Final_Input& operator>>(std::vector<T, Alloc>& x)
	{
		DataIO_load_vector(static_cast<Final_Input&>(*this), x, DATA_IO_BSWAP_FOR_LITTLE(T)());
		return static_cast<Final_Input&>(*this);
	}
};

#ifdef BOOST_BIG_ENDIAN
#  define LittleEndianStringInput BigEndianStringInput
#else
#  define LittleEndianStringInput LittleEndianStringInput
#endif
#define PortableStringInput BigEndianStringInput

template<class StreamT>
class LittleEndianDataInput : public DATA_IO_BASE_IO(LittleEndianDataInput, LittleEndian, VarIntVar, Input, StreamT)
{
public:
	LittleEndianDataInput() {}
};

template<class StreamT>
class LittleEndianDataInput<StreamT*> : public DATA_IO_BASE_IO(LittleEndianDataInput, LittleEndian, VarIntVar, Input, StreamT*)
{
public:
	explicit LittleEndianDataInput(StreamT* stream = 0) {	this->attach(stream); }
};

template<class StreamT>
class PortableDataInput : public DATA_IO_BASE_IO(PortableDataInput, Portable, VarIntVar, Input, StreamT)
{
public:
	PortableDataInput() {}
};

template<class StreamT>
class PortableDataInput<StreamT*> : public DATA_IO_BASE_IO(PortableDataInput, Portable, VarIntVar, Input, StreamT*)
{
public:
	explicit PortableDataInput(StreamT* stream = 0) { this->attach(stream); }
};

//////////////////////////////////////////////////////////////////////////
template<class StreamT>
class LittleEndianNoVarIntInput : public DATA_IO_BASE_IO(LittleEndianNoVarIntInput, LittleEndian, VarIntFixed, Input, StreamT)
{
public:
	LittleEndianNoVarIntInput() {}
};

template<class StreamT>
class LittleEndianNoVarIntInput<StreamT*> : public DATA_IO_BASE_IO(LittleEndianNoVarIntInput, LittleEndian, VarIntFixed, Input, StreamT*)
{
public:
	explicit LittleEndianNoVarIntInput(StreamT* stream) { this->stream = stream; }
};

template<class StreamT>
class PortableNoVarIntInput : public DATA_IO_BASE_IO(PortableNoVarIntInput, Portable, VarIntFixed, Input, StreamT)
{
public:
	PortableNoVarIntInput() {}
};

template<class StreamT>
class PortableNoVarIntInput<StreamT*> : public DATA_IO_BASE_IO(PortableNoVarIntInput, Portable, VarIntFixed, Input, StreamT*)
{
public:
	explicit PortableNoVarIntInput(StreamT* stream = 0) { this->stream = stream; }
};

//////////////////////////////////////////////////////////////////////////

//! call Class::load(Input, version)
template<class Input>
unsigned int DataIO_load_check_version(Input& in, unsigned int curr_version, const char* className)
{
	serialize_version_t loaded_version;
	in >> loaded_version;
	if (loaded_version.t > curr_version)
	{
// 		if (0 == className)
// 			className = typeid(Class).name();
		throw BadVersionException(loaded_version.t, curr_version, className);
	}
	return loaded_version.t;
}

#define DATA_IO_REG_LOAD(Class) \
  template<class Input> friend void DataIO_loadObject(Input& in, Class& x) { x.load(in); }

#define DATA_IO_REG_LOAD_V(Class, CurrentVersion)			\
	template<class Input>									\
	friend void DataIO_loadObject(Input& in, Class& x)		\
	{														\
		x.load(in, DataIO_load_check_version(				\
			in, CurrentVersion, BOOST_STRINGIZE(Class)));	\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_DISABLE_LOAD(Class) \
  template<class DataIO>		\
  friend void DataIO_loadObject(DataIO& dio, Class& x) { dio.DisableLoadClass(x); }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef FEBIRD_DATA_IO_DISABLE_OPTIMIZE_DUMPABLE
#define DATA_IO_OPTIMIZE_VECTOR_LOAD(Self, Class, Members)
#define DATA_IO_OPTIMIZE_ELEMEN_LOAD(Self, Class, Members)
#define DATA_IO_OPTIMIZE_ARRAY_LOAD(Self, Class, Members)
#else
#define DATA_IO_OPTIMIZE_VECTOR_LOAD(Derived, Class, Members)\
	template<class DataIO, class Alloc, class Bswap>\
	void load_vector(DataIO& aDataIO,				\
				std::vector<Class, Alloc>& _vector_,\
				Bswap bswap)						\
	{												\
DataIO_load_vector_opt(aDataIO,_vector_, Bswap(),	\
DataIO_is_realdump<DataIO,Class,0,true>(this)Members);\
	}												\
	template<class DataIO, class Alloc, class Bswap>\
	friend void DataIO_load_vector(DataIO& dio,		\
				std::vector<Class, Alloc>& x,		\
				Bswap bswap)						\
	{												\
		((Derived*)0)->load_vector(dio, x, bswap);	\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_OPTIMIZE_ARRAY_LOAD(Derived, Class, Members) \
	template<class DataIO, class Bswap>				\
	void load_array(DataIO& aDataIO,				\
		Class* _A_address, size_t _N_count,	Bswap)	\
	{												\
	  DataIO_load_array_opt(aDataIO,				\
		_A_address, _N_count, Bswap(),				\
		DataIO_is_realdump<DataIO,Class,0,true>(this)Members);\
	}												\
	template<class DataIO, class Bswap>				\
	friend void DataIO_load_array(DataIO& dio,		\
				Class* a, size_t n,	Bswap bswap)	\
	{												\
		((Derived*)0)->load_array(dio, a, n, bswap);\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_OPTIMIZE_ELEMEN_LOAD(Self, Class, Members)\
	template<class Bswap> void byte_swap_in(Bswap)	\
	{ ByteSwapChain<Bswap>()Members; }				\
	template<class Bswap> friend					\
	void byte_swap_in(Class& x, Bswap)				\
	{ Self(x).byte_swap_in(Bswap()); }				\
	template<class DataIO, class Bswap>				\
	void opt_load(DataIO& aDataIO, Bswap)			\
	{												\
	  DataIO_load_elem_opt(aDataIO, *this, Bswap(), \
		DataIO_is_realdump<DataIO,Class,0,true>(this)Members);\
	}												\
	template<class DataIO, class Bswap>				\
	friend void DataIO_load_elem(DataIO& dio,		\
				Class& x, Bswap bswap)				\
	{												\
		Self(x).opt_load(dio, bswap);				\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#endif // FEBIRD_DATA_IO_DISABLE_OPTIMIZE_DUMPABLE

#ifdef BOOST_LITTLE_ENDIAN
	#define NativeDataInput LittleEndianDataInput
	#define NativeNoVarIntInput LittleEndianNoVarIntInput
#elif defined(BOOST_BIG_ENDIAN)
	#define NativeDataInput PortableDataInput
	#define NativeNoVarIntInput PortableNoVarIntInput
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif


}

#endif // __febird_io_DataInput_h__


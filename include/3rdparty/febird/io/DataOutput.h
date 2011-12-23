/* vim: set tabstop=4 : */
#ifndef __febird_io_DataOutput_h__
#define __febird_io_DataOutput_h__

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
#include <boost/ref.hpp>

#if !defined(BOOST_BIG_ENDIAN) && !defined(BOOST_LITTLE_ENDIAN)
	#error must define byte endian
#endif

#include "../pass_by_value.h"
#include "byte_swap.h"
#include "DataIO_Basic.h"
#include "DataIO_Version.h"
#include "DataIO_Tuple.h"
#include "var_int.h"
#include "IOException.h"

#if !defined(BOOST_BIG_ENDIAN) && !defined(BOOST_LITTLE_ENDIAN)
	#error must define byte endian
#endif

namespace febird {

template<class Output, class T>
void DataIO_saveObject(Output& output, const T& x)
{
#if 1//DATA_IO_ALLOW_DEFAULT_SERIALIZE
	output.ensureWrite(&x, sizeof(x));
#else
	x.MustDefineCustomSave(output);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template<class DataIO, class T, class Alloc>
void DataIO_save_vector_raw(DataIO& dio, const std::vector<T, Alloc>& x, boost::mpl::false_ bswap)
{
	dio.ensureWrite(&*x.begin(), sizeof(T) * x.size());
}

template<class DataIO, class T, class Alloc>
void DataIO_save_vector_raw(DataIO& dio, const std::vector<T, Alloc>& x, boost::mpl::true_ bswap)
{
	for (typename std::vector<T, Alloc>::const_iterator i = x.begin(); i != x.end(); ++i)
	{
		T e(*i);
		byte_swap_in(e, bswap);
		dio.ensureWrite(&e, sizeof(T));
	}
}

template<class DataIO, class T, class Alloc, class Bswap>
void DataIO_save_vector_aux(DataIO& dio, const std::vector<T, Alloc>& x,
							Bswap bswap, ::boost::mpl::true_ real_dumpable)
{
	DataIO_save_vector_raw(dio, x, bswap);
}

template<class DataIO, class T, class Alloc, class Bswap>
void DataIO_save_vector_aux(DataIO& dio, const std::vector<T, Alloc>& x,
							Bswap bswap, ::boost::mpl::false_ real_dumpable)
{
	for (typename std::vector<T, Alloc>::const_iterator i = x.begin(); i != x.end(); ++i)
	{
		dio << *i;
	}
}

template<class DataIO, class T, class Alloc, int Size, bool Dumpable, class Bswap>
void DataIO_save_vector_opt(DataIO& dio, const std::vector<T, Alloc>& x,
							Bswap bswap, DataIO_is_realdump<DataIO, T, Size, Dumpable> tag)
{
	BOOST_STATIC_ASSERT(sizeof(T) >= Size);
	typedef ::boost::mpl::bool_<sizeof(T)==Size && Dumpable> is_realdump_t;
	DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, is_realdump_t);
	dio << var_uint32_t(x.size());
	if (!x.empty())
		DataIO_save_vector_aux(dio, x, bswap, is_realdump_t());
}

template<class DataIO, class T1, class T2, class Alloc, class Bswap>
void DataIO_save_vector(DataIO& dio, const std::vector<std::pair<T1,T2>, Alloc>& x, Bswap bswap)
{
	DataIO_is_realdump<
		DataIO,
		std::pair<T1,T2>,
		sizeof(T1)+sizeof(T2),
		DataIO_is_dump<DataIO, std::pair<T1,T2> >::value
	> tag(NULL);
	DataIO_save_vector_opt(dio, x, bswap, tag);
}

template<class DataIO, class T, class Alloc, class Bswap>
void DataIO_save_vector(DataIO& dio, const std::vector<T, Alloc>& x, Bswap bswap)
{
	DataIO_is_realdump<
		DataIO,
		T,
		sizeof(T),
		DataIO_is_dump<DataIO, T>::value
	> tag(NULL);
	DataIO_save_vector_opt(dio, x, bswap, tag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<class DataIO, class T>
void DataIO_save_array_raw(DataIO& dio, const T* a, size_t n, boost::mpl::false_ bswap)
{
	dio.ensureWrite(a, sizeof(T) * n);
}

template<class DataIO, class T>
void DataIO_save_array_raw(DataIO& dio, const T* a, size_t n, boost::mpl::true_ bswap)
{
	for (size_t i = n; i; --i, ++a)
	{
		T e(*a);
		byte_swap_in(e, bswap);
		dio.ensureWrite(&e, sizeof(T));
	}
}

template<class DataIO, class T, class Bswap>
void DataIO_save_array_aux(DataIO& dio, const T* a, size_t n, Bswap bswap,
					   ::boost::mpl::true_ real_dumpable)
{
	DataIO_save_array_raw(dio, a, n, bswap);
}

template<class DataIO, class T, class Bswap>
void DataIO_save_array_aux(DataIO& dio, const T* a, size_t n, Bswap bswap,
					   ::boost::mpl::false_ real_dumpable)
{
	for (size_t i = n; i; --i, ++a)
		dio << *a;
}

template<class DataIO, class T, int Size, bool Dumpable, class Bswap>
void DataIO_save_array_opt(DataIO& dio, const T* a, size_t n, Bswap bswap,
							 DataIO_is_realdump<DataIO, T, Size, Dumpable> tag)
{
	// sizeof(T) == Size implies T has no paddings
	BOOST_STATIC_ASSERT(sizeof(T) >= Size);
	typedef ::boost::mpl::bool_<sizeof(T)==Size && Dumpable> is_realdump_t;
	DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, is_realdump_t);
	DataIO_save_array_aux(dio, a, n, bswap, is_realdump_t());
}

template<class DataIO, class T1, class T2, class Bswap>
void DataIO_save_array(DataIO& dio, const std::pair<T1,T2>* a, size_t n, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		std::pair<T1,T2>,
		sizeof(T1)+sizeof(T2),
		DataIO_is_dump<DataIO, std::pair<T1,T2> >::value
	> tag(NULL);
	DataIO_save_array_opt(dio, a, n, bs, tag);
}

template<class DataIO, class T, class Bswap>
void DataIO_save_array(DataIO& dio, const T* a, size_t n, Bswap bs)
{
	DataIO_is_realdump<
		DataIO,
		T,
		sizeof(T),
		DataIO_is_dump<DataIO, T>::value
	> tag(NULL);
	DataIO_save_array_opt(dio, a, n, bs, tag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<class DataIO, class T>
void DataIO_save_elem_raw(DataIO& dio, const T& x, ::boost::mpl::false_ bswap)
{
	dio.ensureWrite(&x, sizeof(T));
}

template<class DataIO, class T>
void DataIO_save_elem_raw(DataIO& dio, const T& x, ::boost::mpl::true_ bswap)
{
	T e(x);
	byte_swap_in(e, bswap);
	dio.ensureWrite(&e, sizeof(T));
}

template<class DataIO, class T, class Bswap>
void DataIO_save_elem_aux(DataIO& dio, const T& x, Bswap bswap, boost::mpl::true_ real_dumpable)
{
	DataIO_save_elem_raw(dio, x, bswap);
}

template<class DataIO, class T, class Bswap>
void DataIO_save_elem_aux(DataIO& dio, const T& x, Bswap bswap, boost::mpl::false_ real_dumpable)
{
	DataIO_saveObject(dio, x);
}

template<class DataIO, class T, int Size, bool Dumpable, class Bswap>
void DataIO_save_elem_opt(DataIO& dio, const T& x, Bswap bswap,
						  DataIO_is_realdump<DataIO, T, Size, Dumpable> tag)
{
	BOOST_STATIC_ASSERT(sizeof(T) >= Size);
	typedef ::boost::mpl::bool_<sizeof(T)==Size && Dumpable> is_realdump_t;
	DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, is_realdump_t);
	DataIO_save_elem_aux(dio, x, bswap, is_realdump_t());
}

template<class DataIO, class T, class Bswap>
void DataIO_save_elem(DataIO& dio, const T& x, Bswap bswap)
{
	DataIO_is_realdump<
		DataIO,
		T,
		sizeof(T),
		DataIO_is_dump<DataIO, T>::value
	> tag(NULL);
	DataIO_save_elem_opt(dio, x, bswap, tag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//! string in file format: [length : ....content.... ]
template<class PrimitiveOutput, class CharType, class Traits, class Allocator>
void PrimitiveOutput_saveString(PrimitiveOutput& output, const std::basic_string<CharType, Traits, Allocator>& str)
{
	var_uint32_t length(str.size());
	output << (length);
	output.save(str.data(), length.t);
}

//! @brief Primitive Output save string
//!
//! string in file format: [length : ....content.... ]
//! @param[in]  buffer       pointer buffer
//! @param[in]  length       buffer length
template<class PrimitiveOutput, class CharType>
void PrimitiveOutput_saveString(PrimitiveOutput& output, const CharType* buffer, size_t length)
{
	assert(0 != buffer);
	output.save(buffer, length);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template<class BaseOutput, class Final_Output>
class DataOutput : public BaseOutput
{
public:
	typedef boost::mpl::false_ is_loading;
	typedef boost::mpl::true_  is_saving;

	using BaseOutput::operator<<;

	//////////////////////////////////////////////////////////////////////////
	//! dual operator, for auto dispatch single serialize proc...
	//!
	//! for PrimitiveOutputImpl, operator& is output,
	//! for DataOutput, operator& is output.
	template<class T> Final_Output& operator&(const T& x) { return operator<<(x); }

	template<class T> Final_Output& operator& (pass_by_value<T> x) { return operator<<(x.val); }
	template<class T> Final_Output& operator<<(pass_by_value<T> x) { return operator<<(x.val); }

	template<class T> Final_Output& operator& (boost::reference_wrapper<T> x) { return operator<<(x.get()); }
	template<class T> Final_Output& operator<<(boost::reference_wrapper<T> x) { return operator<<(x.get()); }

	template<int Dim> Final_Output& operator<<(const char (&x)[Dim]) { return this->save(x, Dim); }
	template<int Dim> Final_Output& operator<<(const byte (&x)[Dim]) { return this->save(x, Dim); }

#ifdef DATA_IO_SUPPORT_SERIALIZE_PTR
	template<class T> Final_Output& operator<<(T*& x)
	{
		*this << *x;
		return static_cast<Final_Output&>(*this);
	}
	template<class T> Final_Output& operator<<(const T*& x)
	{
		*this << *x;
		return static_cast<Final_Output&>(*this);
	}
#else
	template<class T> Final_Output& operator<<(T*& x)
	{
		T::NotSupportSerializePointer();
		return static_cast<Final_Output&>(*this);
	}
	template<class T> Final_Output& operator<<(const T*& x)
	{
		T::NotSupportSerializePointer();
		return static_cast<Final_Output&>(*this);
	}
#endif

	template<class Container>
	Final_Output& container_save(const Container& x)
	{
		var_uint32_t size(x.size());
		*this << size;
		for (typename Container::const_iterator i = x.begin(); i != x.end(); ++i)
		{
			*this << *i;
		}
		return static_cast<Final_Output&>(*this);
	}

	//!@{
	//! standard container output.....
	template<class KeyT, class ValueT, class CompareT, class AllocT>
	Final_Output& operator<<(const std::map<KeyT, ValueT, CompareT, AllocT>& x)
	{
		return container_save(x);
	}
	template<class KeyT, class ValueT, class CompareT, class AllocT>
	Final_Output& operator<<(const std::multimap<KeyT, ValueT, CompareT, AllocT>& x)
	{
		return container_save(x);
	}
	template<class ValueT, class CompareT, class AllocT>
	Final_Output& operator<<(const std::set<ValueT, CompareT, AllocT>& x)
	{
		return container_save(x);
	}
	template<class ValueT, class CompareT, class AllocT>
	Final_Output& operator<<(const std::multiset<ValueT, CompareT, AllocT>& x)
	{
		return container_save(x);
	}
	template<class ValueT, class AllocT>
	Final_Output& operator<<(const std::list<ValueT, AllocT>& x)
	{
		return container_save(x);
	}
	template<class ValueT, class AllocT>
	Final_Output& operator<<(const std::deque<ValueT, AllocT>& x)
	{
		return container_save(x);
	}
};

//////////////////////////////////////////////////////////////////////////
template<class BaseOutput, class Final_Output>
class VarIntVarOutput : public BaseOutput
{
public:
	using BaseOutput::operator<<;

	Final_Output& operator<<(var_int16_t x)
	{
		byte buf[3];
		this->ensureWrite(buf, save_var_int16(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}
	Final_Output& operator<<(var_int32_t x)
	{
		byte buf[5];
		this->ensureWrite(buf, save_var_int32(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}

	Final_Output& operator<<(var_uint16_t x)
	{
		byte buf[3];
		this->ensureWrite(buf, save_var_uint16(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}
	Final_Output& operator<<(var_uint32_t x)
	{
		byte buf[5];
		this->ensureWrite(buf, save_var_uint32(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}

#if !defined(BOOST_NO_INT64_T)
	Final_Output& operator<<(var_int64_t x)
	{
		byte buf[9];
		this->ensureWrite(buf, save_var_int64(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}
	Final_Output& operator<<(var_uint64_t x)
	{
		byte buf[9];
		this->ensureWrite(buf, save_var_uint64(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}
#endif
	Final_Output& operator<<(serialize_version_t x)
	{
		byte buf[5];
		this->ensureWrite(buf, save_var_uint32(buf, x.t) - buf);
		return static_cast<Final_Output&>(*this);
	}
};

template<class BaseOutput, class Final_Output>
class VarIntFixedOutput : public BaseOutput
{
public:
	using BaseOutput::operator<<;
	Final_Output& operator<<(var_int16_t  x) { return this->operator<<(x.t); }
	Final_Output& operator<<(var_uint16_t x) { return this->operator<<(x.t); }

	Final_Output& operator<<(var_int32_t  x) { return this->operator<<(x.t); }
	Final_Output& operator<<(var_uint32_t x) { return this->operator<<(x.t); }

#if !defined(BOOST_NO_INT64_T)
	Final_Output& operator<<(var_int64_t  x) { return this->operator<<(x.t); }
	Final_Output& operator<<(var_uint64_t x) { return this->operator<<(x.t); }
#endif

	Final_Output& operator<<(serialize_version_t x) { return this->operator<<(x.t); }
};

template<class BaseOutput, class Final_Output>
class BigEndianStringOutput : public BaseOutput
{
public:
	using BaseOutput::operator<<;

	Final_Output& save(const wchar_t* s, size_t n)
	{
#ifdef BOOST_BIG_ENDIAN
		this->ensureWrite(s, sizeof(wchar_t)*n);
#else
		std::vector<wchar_t> tempv(s, s + n);
		byte_swap(&*tempv.begin(), n);
		this->ensureWrite(&*tempv.begin(), sizeof(wchar_t)*n);
#endif
		return static_cast<Final_Output&>(*this);
	}
#ifndef BOOST_NO_INTRINSIC_WCHAR_T
	Final_Output& operator<<(wchar_t x)
	{
#ifdef BOOST_LITTLE_ENDIAN
		x = byte_swap(x);
#endif
		this->ensureWrite(&x, sizeof(x));
		return static_cast<Final_Output&>(*this);
	}
#endif
};

template<class BaseOutput, class Final_Output>
class LittleEndianStringOutput : public BaseOutput
{
public:
	using BaseOutput::operator<<;

	Final_Output& save(const wchar_t* s, size_t n)
	{
#ifdef BOOST_LITTLE_ENDIAN
		this->ensureWrite(s, sizeof(wchar_t)*n);
#else
		std::vector<wchar_t> tempv(s, s + n);
		byte_swap(&*tempv.begin(), n);
		this->ensureWrite(&*tempv.begin(), sizeof(wchar_t)*n);
#endif
		return static_cast<Final_Output&>(*this);
	}
#ifndef BOOST_NO_INTRINSIC_WCHAR_T
	Final_Output& operator<<(wchar_t x)
	{
#ifdef BOOST_BIG_ENDIAN
		x = byte_swap(x);
#endif
		this->ensureWrite(&x, sizeof(x));
		return static_cast<Final_Output&>(*this);
	}
#endif
};

template<class BaseOutput, class Final_Output>
class CommonStringOutput : public BaseOutput
{
public:
	using BaseOutput::save;
	using BaseOutput::operator<<;

	Final_Output& save(const char* s, size_t n) { this->ensureWrite(s, n); return static_cast<Final_Output&>(*this); }
	Final_Output& save(const unsigned char* s, size_t n) { this->ensureWrite(s, n); return static_cast<Final_Output&>(*this); }
	Final_Output& save(const   signed char* s, size_t n) { this->ensureWrite(s, n); return static_cast<Final_Output&>(*this); }

	Final_Output& operator<<(const char* s)
	{
		var_uint32_t n(strlen(s));
		static_cast<Final_Output&>(*this) << n;
		this->ensureWrite(s, n.t);
		return static_cast<Final_Output&>(*this);
	}
	Final_Output& operator<<(const wchar_t* s)
	{
		var_uint32_t n(wcslen(s));
		static_cast<Final_Output&>(*this) << n;
		this->save(s, n.t);
		return static_cast<Final_Output&>(*this);
	}
#if 0
	template<class CharType, class Traits, class Allocator>
	Final_Output& operator<<(const std::basic_string<CharType, Traits, Allocator>& x)
	{
		var_uint32_t length(x.size());
		*this << (length);
		this->save(x.data(), length.t);
		return static_cast<Final_Output&>(*this);
	}
#else
	Final_Output& operator<<(const std::string& x)
	{
		var_uint32_t length(x.size());
		*this << (length);
		this->save(x.data(), length.t);
		return static_cast<Final_Output&>(*this);
	}
	Final_Output& operator<<(const std::wstring& x)
	{
		var_uint32_t length(x.size());
		*this << (length);
		this->save(x.data(), length.t);
		return static_cast<Final_Output&>(*this);
	}
#endif
};

template<class StreamT, class Final_Output>
class PrimitiveOutputImpl : public StreamT
{
	//  this will cause compile error!!
	//	DECLARE_NONE_COPYABLE_CLASS(PrimitiveOutputImpl)
public:
	typedef StreamT stream_t;

	StreamT* getStream() { return this; }

	Final_Output& operator<<(char x) { this->writeByte((byte)x); return static_cast<Final_Output&>(*this); }
	Final_Output& operator<<(unsigned char x) { this->writeByte((byte)x); return static_cast<Final_Output&>(*this); }
	Final_Output& operator<<(  signed char x) { this->writeByte((byte)x); return static_cast<Final_Output&>(*this); }
};

template<class StreamT, class Final_Output>
class PrimitiveOutputImpl<StreamT*, Final_Output>
{
protected:
	StreamT* stream;

public:
	typedef StreamT stream_t;
	typedef typename StreamT::is_seekable is_seekable;

	void attach(StreamT* stream) { this->stream = stream; }
	StreamT* getStream() const { return stream; }

	void flush() { stream->flush(); }

	//! delegate these 2 method of StreamT
	size_t write(const void* data, size_t length) { return stream->write(data, length); }
	void ensureWrite(const void* data, size_t length) { stream->ensureWrite(data, length); }

	Final_Output& operator<<(char x) { stream->writeByte((byte)x); return static_cast<Final_Output&>(*this); }
	Final_Output& operator<<(byte x) { stream->writeByte((byte)x); return static_cast<Final_Output&>(*this); }
	Final_Output& operator<<(signed char x) { stream->writeByte((byte)x); return static_cast<Final_Output&>(*this); }
};

#define DATA_IO_GEN_DUMP_OUTPUT(Type)		\
	Final_Output& operator<<(Type x) {		\
		this->ensureWrite(&x, sizeof(Type));\
		return static_cast<Final_Output&>(*this); \
	}										\
	template<int Dim>						\
	Final_Output& operator<<(const Type (&x)[Dim]) {\
		this->ensureWrite(x, sizeof(Type)*Dim);\
		return static_cast<Final_Output&>(*this); \
	}										\
	template<class Alloc>					\
	Final_Output& operator<<(const std::vector<Type, Alloc>& x) {\
		static_cast<Final_Output&>(*this) << var_uint32_t(x.size());\
		if (!x.empty()) \
		  this->ensureWrite(&*x.begin(), sizeof(Type) * x.size());\
	return static_cast<Final_Output&>(*this); \
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_GEN_BSWAP_INT_OUTPUT(Int)\
	Final_Output& operator<<(Int x) {		\
		x = byte_swap(x);					\
		this->ensureWrite(&x, sizeof(Int));	\
		return static_cast<Final_Output&>(*this); \
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<class BaseOutput, class Final_Output>
class BinFloatOutput : public BaseOutput
{
public:
	using BaseOutput::operator<<;

	DATA_IO_GEN_DUMP_OUTPUT(float)
	DATA_IO_GEN_DUMP_OUTPUT(double)
	DATA_IO_GEN_DUMP_OUTPUT(long double)
};

template<class StreamT, class Final_Output>
class PortableIntegerOutput : public PrimitiveOutputImpl<StreamT, Final_Output>
{
public:
	using PrimitiveOutputImpl<StreamT, Final_Output>::operator<<;

#ifdef BOOST_LITTLE_ENDIAN
	#define DATA_IO_GEN_PORTABLE_INT_OUTPUT  DATA_IO_GEN_BSWAP_INT_OUTPUT
#elif defined(BOOST_BIG_ENDIAN)
	#define DATA_IO_GEN_PORTABLE_INT_OUTPUT  DATA_IO_GEN_DUMP_OUTPUT
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif

	DATA_IO_GEN_PORTABLE_INT_OUTPUT(short)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(unsigned short)

	DATA_IO_GEN_PORTABLE_INT_OUTPUT(int)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(unsigned int)

	DATA_IO_GEN_PORTABLE_INT_OUTPUT(long)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(unsigned long)

#if defined(BOOST_HAS_LONG_LONG)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(long long)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(unsigned long long)
#elif defined(BOOST_HAS_MS_INT64)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(__int64)
	DATA_IO_GEN_PORTABLE_INT_OUTPUT(unsigned __int64)
#endif

	template<class T> Final_Output& operator<<(const T& x)
	{
		DataIO_save_elem(static_cast<Final_Output&>(*this), x, DATA_IO_BSWAP_FOR_BIG(T)());
		return static_cast<Final_Output&>(*this);
	}

	template<class T, int Dim>
	Final_Output& operator<<(const T (&x)[Dim])
	{
		DataIO_save_array(static_cast<Final_Output&>(*this), x, Dim, DATA_IO_BSWAP_FOR_BIG(T)());
		return static_cast<Final_Output&>(*this);
	}

	template<class T, class Alloc>
	Final_Output& operator<<(const std::vector<T, Alloc>& x)
	{
		DataIO_save_vector(static_cast<Final_Output&>(*this), x, DATA_IO_BSWAP_FOR_BIG(T)());
		return static_cast<Final_Output&>(*this);
	}
};


template<class StreamT, class Final_Output>
class LittleEndianIntegerOutput : public PrimitiveOutputImpl<StreamT, Final_Output>
{
public:
	using PrimitiveOutputImpl<StreamT, Final_Output>::operator<<;

#ifdef BOOST_LITTLE_ENDIAN
	#define DATA_IO_GEN_LITTLE_INT_OUTPUT  DATA_IO_GEN_DUMP_OUTPUT
#elif defined(BOOST_BIG_ENDIAN)
	#define DATA_IO_GEN_LITTLE_INT_OUTPUT  DATA_IO_GEN_BSWAP_INT_OUTPUT
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif

	DATA_IO_GEN_LITTLE_INT_OUTPUT(short)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(unsigned short)

	DATA_IO_GEN_LITTLE_INT_OUTPUT(int)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(unsigned int)

	DATA_IO_GEN_LITTLE_INT_OUTPUT(long)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(unsigned long)

#if defined(BOOST_HAS_LONG_LONG)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(long long)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(unsigned long long)
#elif defined(BOOST_HAS_MS_INT64)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(__int64)
	DATA_IO_GEN_LITTLE_INT_OUTPUT(unsigned __int64)
#endif

	template<class T> Final_Output& operator<<(const T& x)
	{
		DataIO_save_elem(static_cast<Final_Output&>(*this), x, DATA_IO_BSWAP_FOR_LITTLE(T)());
		return static_cast<Final_Output&>(*this);
	}

	template<class T, int Dim>
	Final_Output& operator<<(const T (&x)[Dim])
	{
		DataIO_save_array(static_cast<Final_Output&>(*this), x, Dim, DATA_IO_BSWAP_FOR_LITTLE(T)());
		return static_cast<Final_Output&>(*this);
	}

	template<class T, class Alloc>
	Final_Output& operator<<(const std::vector<T, Alloc>& x)
	{
		DataIO_save_vector(static_cast<Final_Output&>(*this), x, DATA_IO_BSWAP_FOR_LITTLE(T)());
		return static_cast<Final_Output&>(*this);
	}
};

#define PortableStringOutput BigEndianStringOutput

template<class StreamT>
class LittleEndianDataOutput : public DATA_IO_BASE_IO(LittleEndianDataOutput, LittleEndian, VarIntVar, Output, StreamT)
{
public:
	LittleEndianDataOutput() {}
};

template<class StreamT>
class LittleEndianDataOutput<StreamT*> : public DATA_IO_BASE_IO(LittleEndianDataOutput, LittleEndian, VarIntVar, Output, StreamT*)
{
public:
	explicit LittleEndianDataOutput(StreamT* stream) { this->stream = stream; }
};

template<class StreamT>
class PortableDataOutput : public DATA_IO_BASE_IO(PortableDataOutput, Portable, VarIntVar, Output, StreamT)
{
public:
	PortableDataOutput() {}
};

template<class StreamT>
class PortableDataOutput<StreamT*> : public DATA_IO_BASE_IO(PortableDataOutput, Portable, VarIntVar, Output, StreamT*)
{
public:
	explicit PortableDataOutput(StreamT* stream = 0) { this->stream = stream; }
};

//////////////////////////////////////////////////////////////////////////
template<class StreamT>
class LittleEndianNoVarIntOutput : public DATA_IO_BASE_IO(LittleEndianNoVarIntOutput, LittleEndian, VarIntFixed, Output, StreamT)
{
public:
	LittleEndianNoVarIntOutput() {}
};

template<class StreamT>
class LittleEndianNoVarIntOutput<StreamT*> : public DATA_IO_BASE_IO(LittleEndianNoVarIntOutput, LittleEndian, VarIntFixed, Output, StreamT*)
{
public:
	explicit LittleEndianNoVarIntOutput(StreamT* stream) { this->stream = stream; }
};

template<class StreamT>
class PortableNoVarIntOutput : public DATA_IO_BASE_IO(PortableNoVarIntOutput, Portable, VarIntFixed, Output, StreamT)
{
public:
	PortableNoVarIntOutput() {}
};

template<class StreamT>
class PortableNoVarIntOutput<StreamT*> : public DATA_IO_BASE_IO(PortableNoVarIntOutput, Portable, VarIntFixed, Output, StreamT*)
{
public:
	explicit PortableNoVarIntOutput(StreamT* stream = 0) { this->stream = stream; }
};

//////////////////////////////////////////////////////////////////////////


template<class StreamClass>
LittleEndianDataOutput<StreamClass*> LittleEndianDataOutputer(StreamClass* stream)
{
	return LittleEndianDataOutput<StreamClass*>(stream);
}

template<class StreamClass>
PortableDataOutput<StreamClass*> PortableDataOutputer(StreamClass* stream)
{
	return PortableDataOutput<StreamClass*>(stream);
}

//////////////////////////////////////////////////////////////////////////

template<class Output, class FirstT, class SecondT>
void DataIO_saveObject(Output& output, const std::pair<FirstT, SecondT>& x)
{
	output << x.first << x.second;
}

#define DATA_IO_REG_SAVE(Class) \
  template<class Output> friend void DataIO_saveObject(Output& output, const Class& x) { x.save(output); }

#define DATA_IO_REG_SAVE_V(Class, CurrentVersion)	\
	template<class Output>										\
	friend void DataIO_saveObject(Output& out, const Class& x)	\
	{															\
		out << febird::serialize_version_t(CurrentVersion);		\
		x.save(out, CurrentVersion);							\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_DISABLE_SAVE(Class) \
  template<class DataIO>		\
  friend void DataIO_saveObject(DataIO& dio, const Class& x) { dio.DisableSaveClass(x); }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifdef FEBIRD_DATA_IO_DISABLE_OPTIMIZE_DUMPABLE

#define DATA_IO_OPTIMIZE_VECTOR_SAVE(Self, Class, Members)
#define DATA_IO_OPTIMIZE_ELEMEN_SAVE(Self, Class, Members)
#define DATA_IO_OPTIMIZE_ARRAY_SAVE(Self, Class, Members)

#else

//! 宏中的成员函数使用较长的参数名，目的是避免和 Members 中的某个成员同名
//!
#define DATA_IO_OPTIMIZE_VECTOR_SAVE(Derived, Class, Members)\
	template<class DataIO, class Alloc, class Bswap>\
	void save_vector(DataIO& aDataIO,				\
		const std::vector<Class, Alloc>& _vector_,	\
						Bswap)						\
	{												\
		DataIO_save_vector_opt(						\
			aDataIO, _vector_, Bswap(),				\
	DataIO_is_realdump<DataIO,Class,0,true>(this)Members);\
	}												\
	template<class DataIO, class Alloc, class Bswap>\
	friend void DataIO_save_vector(DataIO& dio,		\
				const std::vector<Class, Alloc>& x,	\
				Bswap bswap)						\
	{												\
		((Derived*)0)->save_vector(dio, x, bswap);	\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_OPTIMIZE_ARRAY_SAVE(Derived, Class, Members) \
	template<class DataIO, class Bswap>				\
	void save_array(DataIO& aDataIO					\
		, const Class* _A_address, size_t _N_count	\
		, Bswap)									\
	{												\
		DataIO_save_array_opt(						\
			aDataIO, _A_address, _N_count, Bswap(),	\
	DataIO_is_realdump<DataIO,Class,0,true>(this)Members);\
	}												\
	template<class DataIO, class Bswap>				\
	friend void DataIO_save_array(DataIO& dio		\
		, const Class* a, size_t n, Bswap bswap)	\
	{												\
		((Derived*)0)->save_array(dio, a, n, bswap);\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define DATA_IO_OPTIMIZE_ELEMEN_SAVE(Self, Class, Members)\
	template<class DataIO, class Bswap>				\
	void opt_save(DataIO& aDataIO, Bswap) const		\
	{												\
	DataIO_save_elem_opt(aDataIO, *this, Bswap(),	\
	DataIO_is_realdump<DataIO,Class,0,true>(this)Members);\
	}												\
	template<class DataIO, class Bswap> friend		\
	void DataIO_save_elem(DataIO& dio,				\
			const Class& x, Bswap bswap)			\
	{												\
		Self(x).opt_save(dio, bswap);				\
	}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#endif // FEBIRD_DATA_IO_DISABLE_OPTIMIZE_DUMPABLE

#ifdef BOOST_LITTLE_ENDIAN
	#define NativeDataOutput LittleEndianDataOutput
	#define NativeNoVarIntOutput LittleEndianNoVarIntOutput
#elif defined(BOOST_BIG_ENDIAN)
	#define NativeDataOutput PortableDataOutput
	#define NativeNoVarIntOutput PortableNoVarIntOutput
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif

} // namespace febird


#endif // __febird_io_DataOutput_h__


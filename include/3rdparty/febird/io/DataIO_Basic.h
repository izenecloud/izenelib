/* vim: set tabstop=4 : */
#ifndef __febird_io_DataIO_Basic_h__
#define __febird_io_DataIO_Basic_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <utility>

#ifndef BOOST_INTRUSIVE_PTR_HPP_INCLUDED
#  include <boost/intrusive_ptr.hpp>
#endif
#ifndef BOOST_SCOPED_PTR_HPP_INCLUDED
#  include <boost/scoped_ptr.hpp>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#  include <boost/shared_ptr.hpp>
#endif

#if BOOST_VERSION < 103301
# include <boost/limits.hpp>
# include <boost/detail/limits.hpp>
#else
# include <boost/detail/endian.hpp>
#endif
#include <boost/cstdint.hpp>
#include <boost/mpl/bool.hpp>
#include "var_int.h"
//#include <boost/type_traits/detail/bool_trait_def.hpp>

namespace febird {

//////////////////////////////////////////////////////////////////////////

#ifdef BOOST_LITTLE_ENDIAN
	#define DATA_IO_BSWAP_FOR_BIG(T)    typename DataIO_need_bswap<T>::type
	#define DATA_IO_BSWAP_FOR_LITTLE(T) boost::mpl::false_
#elif defined(BOOST_BIG_ENDIAN)
	#define DATA_IO_BSWAP_FOR_BIG(T)    boost::mpl::false_
	#define DATA_IO_BSWAP_FOR_LITTLE(T) typename DataIO_need_bswap<T>::type
#else
	#error "must define BOOST_LITTLE_ENDIAN or BOOST_BIG_ENDIAN"
#endif

#if 0
BOOST_TT_AUX_BOOL_TRAIT_DEF2(DataIO_is_dump, DataIO, T, false)

#define DataIO_IsDump_TypeTrue1(T) \
	BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(class DataIO, DataIO_is_dump, DataIO, T, true)

#define DataIO_IsDump_TypeTrue2(ByteOrder, T) \
	BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(class Stream, DataIO_is_dump, ByteOrder##Input<Stream>, T, true) \
	BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(class Stream, DataIO_is_dump, ByteOrder##Output<Stream>, T, true)

// support std::pair
// same as expand of BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1
// because one macro arg can not take unparented ','
template<class DataIO, class X, class Y>
struct DataIO_is_dump<DataIO, std::pair<X,Y> >
	BOOST_TT_AUX_BOOL_C_BASE((DataIO_is_dump<DataIO, X>::value && DataIO_is_dump<DataIO, Y>::value) && sizeof(std::pair<X,Y>) == sizeof(X) + sizeof(Y))
{
    BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL((DataIO_is_dump<DataIO, X>::value && DataIO_is_dump<DataIO, Y>::value) && sizeof(std::pair<X,Y>) == sizeof(X) + sizeof(Y))
};
#endif

//! default false
template<class DataIO, class T>
struct DataIO_is_dump : public ::boost::mpl::false_
{
};

#if defined(FEBIRD_DATA_IO_DISABLE_OPTIMIZE_DUMPABLE)

	#define DataIO_IsDump_TypeTrue1(T)
	#define DataIO_IsDump_TypeTrue2(ByteOrder, T)

#else

	#define DataIO_IsDump_TypeTrue1(T) \
	template<class DataIO> struct DataIO_is_dump<DataIO, T> : public ::boost::mpl::true_ {};

	#define DataIO_IsDump_TypeTrue2(ByteOrder, T)		\
	template<class Stream>								\
	struct DataIO_is_dump<ByteOrder##Input<Stream>, T>	\
		: public ::boost::mpl::true_ {};				\
	template<class Stream>								\
	struct DataIO_is_dump<ByteOrder##Output<Stream>, T>	\
		: public ::boost::mpl::true_ {};				\
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif // FEBIRD_DATA_IO_DISABLE_OPTIMIZE_DUMPABLE

DataIO_IsDump_TypeTrue1(float)
DataIO_IsDump_TypeTrue1(double)
DataIO_IsDump_TypeTrue1(long double)

DataIO_IsDump_TypeTrue1(char)
DataIO_IsDump_TypeTrue1(signed char)
DataIO_IsDump_TypeTrue1(unsigned char)

DataIO_IsDump_TypeTrue1(short)
DataIO_IsDump_TypeTrue1(unsigned short)
DataIO_IsDump_TypeTrue1(int)
DataIO_IsDump_TypeTrue1(unsigned int)
DataIO_IsDump_TypeTrue1(long)
DataIO_IsDump_TypeTrue1(unsigned long)
#if defined(BOOST_HAS_LONG_LONG)
DataIO_IsDump_TypeTrue1(long long)
DataIO_IsDump_TypeTrue1(unsigned long long)
#elif defined(BOOST_HAS_MS_INT64)
DataIO_IsDump_TypeTrue1(__int64)
DataIO_IsDump_TypeTrue1(unsigned __int64)
#endif

// support std::pair
template<class DataIO, class X, class Y>
struct DataIO_is_dump<DataIO, std::pair<X,Y> > :
	public ::boost::mpl::bool_<DataIO_is_dump<DataIO, X>::value &&
							   DataIO_is_dump<DataIO, Y>::value &&
							   sizeof(std::pair<X,Y>) == sizeof(X)+sizeof(Y)>
{
};

template<class DataIO, class T, int Dim>
struct DataIO_is_dump<DataIO, T[Dim]> : public DataIO_is_dump<DataIO, T>::type
{
};

template<class DataIO, class T, int Dim>
struct DataIO_is_dump<DataIO, const T[Dim]> : public DataIO_is_dump<DataIO, T>::type
{
};

//////////////////////////////////////////////////////////////////////////
template<class DataIO, class Outer, int Size, bool Dumpable>
struct DataIO_is_realdump
{
	BOOST_STATIC_CONSTANT(int, size = Size);
	typedef boost::mpl::bool_<Dumpable> is_dump;

#if (defined(_DEBUG) || !defined(NDEBUG)) && !defined(DATA_IO_DONT_CHECK_REAL_DUMP)
	const Outer* address;
	const char*  prev_member;
	DataIO_is_realdump(const Outer* p, const void* prev_member)
		: address(p), prev_member((const char*)prev_member) {}
	DataIO_is_realdump(const Outer* p)
		: address(p), prev_member((const char*)p) {}
#else
	DataIO_is_realdump(const Outer*, const void* = 0) {}
#endif

#if (defined(_DEBUG) || !defined(NDEBUG)) && !defined(DATA_IO_DONT_CHECK_REAL_DUMP)
	template<class T>
	void check_member_order(const T& x, ::boost::mpl::true_)
	{
	// if member declaration order of &a&b&c&d is different with where they defined in class
	// here will raise an assertion fail
	//
	// 如果成员序列化声明的顺序 &a&b&c&d 和它们在类中定义的顺序不同
	// 这里会引发一个断言
	// 如果序列化了一个非类的成员（比如 &a&b&c&d 中可能一个标识符引用了不是雷成员的某个变量），也会引发断言
		assert((const char*)&x >= (const char*)address);
		assert((const char*)&x <= (const char*)address + sizeof(Outer) - sizeof(T));
		if (prev_member != (const char*)address)
			assert(prev_member < (const char*)&x);
	}
	template<class T>
	void check_member_order(const T&, ::boost::mpl::false_)
	{
	}
#endif

	template<class T>
	DataIO_is_realdump<DataIO, Outer, Size+sizeof(T), boost::mpl::bool_<Dumpable && DataIO_is_dump<DataIO, T>::value>::value>
	operator&(const T& x)
	{
		typedef DataIO_is_realdump<DataIO, Outer, Size+sizeof(T), boost::mpl::bool_<Dumpable && DataIO_is_dump<DataIO, T>::value>::value> ret_t;
#if (defined(_DEBUG) || !defined(NDEBUG)) && !defined(DATA_IO_DONT_CHECK_REAL_DUMP)
		check_member_order(x, ::boost::mpl::bool_<Dumpable && DataIO_is_dump<DataIO, T>::value>());
		return ret_t(address, &x);
#else
		return ret_t(NULL);
#endif
	}
/*
	template<class T, int Dim>
	DataIO_is_realdump<DataIO, Outer, Size+sizeof(T)*Dim, boost::mpl::bool_<Dumpable && DataIO_is_dump<DataIO, T>::value>::value>
	operator&(const T (&x)[Dim])
	{
		typedef DataIO_is_realdump<DataIO, Outer, Size+sizeof(T)*Dim, boost::mpl::bool_<Dumpable && DataIO_is_dump<DataIO, T>::value>::value> ret_t;
#if (defined(_DEBUG) || !defined(NDEBUG)) && !defined(DATA_IO_DONT_CHECK_REAL_DUMP)
		check_member_order(x, ::boost::mpl::bool_<Dumpable && DataIO_is_dump<DataIO, T>::value>());
		return ret_t(address, &x);
#else
		return ret_t(NULL);
#endif
	}
*/
};


// if not real_dumpable, then DataIO_is_dump must be false
// if not real_dumpable, and  DataIO_is_dump is true, this is an error,
// this error maybe caused by user defined a custom DataIO_is_dump trait
//
// if real_dumpable, DataIO_is_dump can be any, because '&' operator can not deduce DataIO_is_dump
#define DATA_IO_OPT_TRAITS_VERIFY(DataIO, T, Real_Dumpable) \
	BOOST_STATIC_ASSERT(Real_Dumpable::value \
					|| (!Real_Dumpable::value && !(DataIO_is_dump<DataIO, T>::value)))

//////////////////////////////////////////////////////////////////////////////////////

#define DATA_IO_BASE_IO(Self, Prefix, VarIntPrefix, IO, Stream) \
Data##IO<CommonString##IO< \
	Prefix##String##IO< \
		BinFloat##IO< \
			VarIntPrefix##IO< \
				Prefix##Integer##IO<Stream, Self<Stream> >, \
				Self<Stream> >, \
			Self<Stream> >, \
		Self<Stream> >, \
	Self<Stream> >, \
Self<Stream> >
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

} // namespace febird

//#include <boost/type_traits/detail/bool_trait_undef.hpp>

#endif // __febird_io_DataIO_Basic_h__


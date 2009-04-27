/* vim: set tabstop=4 : */
#ifndef __febird_io_var_int_h__
#define __febird_io_var_int_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <limits>

#include <boost/config.hpp>
#include <boost/strong_typedef.hpp>
#include <boost/static_assert.hpp>

#include "../stdtypes.h"
#include "../pass_by_value.h"

#include "byte_swap.h"
#include "is_primitive.h"

// should be the last #include
#include <boost/type_traits/detail/bool_trait_def.hpp>

#undef min
#undef max

namespace febird {

template<class VarIntT>
struct var_int_org;

template<class IntT>
struct var_int;

//! is_primitive && is_var_int
//!
#define FEBIRD_DEFINE_VAR_INT(IntT)	\
	BOOST_STRONG_TYPEDEF(IntT, BOOST_JOIN(var_, IntT))	\
	BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_var_int, BOOST_JOIN(var_, IntT), true)	 \
	BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_primitive, BOOST_JOIN(var_, IntT), true) \
	template<> struct var_int<IntT> { typedef BOOST_JOIN(var_, IntT) type; };	 \
	template<> struct var_int<BOOST_JOIN(var_, IntT)> { typedef BOOST_JOIN(var_, IntT) type; };	 \
	template<> struct var_int_org<BOOST_JOIN(var_, IntT)> { typedef IntT type; };\
/**/

// template<class T, class SFINAE = void>
// struct to_var_int
// {
// 	typedef T type;
// };
// template<>
// struct to_var_int<IntT, void>
// {
// 	typedef typename var_int<IntT>::type type;
// };

BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_var_int,T,false)

// var int is  var_+IntT, var_int16_t, var_uint16_t, ...
// var_int<IntT>::org_int is original int of the var int
// is_var_int is the typetrait of var int

FEBIRD_DEFINE_VAR_INT( int16_t)
FEBIRD_DEFINE_VAR_INT(uint16_t)
FEBIRD_DEFINE_VAR_INT( int32_t)
FEBIRD_DEFINE_VAR_INT(uint32_t)

#if !defined(BOOST_NO_INT64_T)
FEBIRD_DEFINE_VAR_INT( int64_t)
FEBIRD_DEFINE_VAR_INT(uint64_t)
#endif
/*
inline void byte_swap_in(var_int16_t& x, boost::mpl::true_) { x.t = byte_swap(x.t); }
inline void byte_swap_in(var_int32_t& x, boost::mpl::true_) { x.t = byte_swap(x.t); }
inline void byte_swap_in(var_int64_t& x, boost::mpl::true_) { x.t = byte_swap(x.t); }
inline void byte_swap_in(var_uint16_t& x, boost::mpl::true_) { x.t = byte_swap(x.t); }
inline void byte_swap_in(var_uint32_t& x, boost::mpl::true_) { x.t = byte_swap(x.t); }
inline void byte_swap_in(var_uint64_t& x, boost::mpl::true_) { x.t = byte_swap(x.t); }
*/
/*
#if ULONG_MAX == 0xffffffff
template<> struct var_int<unsigned long> { typedef var_uint32_t type; };
template<> struct var_int<long> { typedef var_int32_t type; };
#endif

#if UINT_MAX == 0xffffffff
template<> struct var_int<unsigned int> { typedef var_uint32_t type; };
template<> struct var_int<int> { typedef var_int32_t type; };
#endif
*/
//#undef FEBIRD_DEFINE_VAR_INT

template<class IntT>
class as_var_int_ref
{
	IntT& val;

public:
	explicit as_var_int_ref(IntT& x) : val(x) {}

	template<class Input>
	friend void DataIO_loadObject(Input& in, as_var_int_ref x)
	{
		typename var_int<IntT>::type v;
		in >> v;
		x.val = v.t;
	}
	template<class Output>
	friend void DataIO_saveObject(Output& out, as_var_int_ref x)
	{
		// 必须有这个函数，因为有可能把一个 non-const int& 传递给 as_var_int 来输出
		out << typename var_int<IntT>::type(x.val);
	}
};

//! for load as var int
template<class IntT>
inline
pass_by_value<as_var_int_ref<IntT> >
as_var_int(IntT& x)
{
	return pass_by_value<as_var_int_ref<IntT> >(as_var_int_ref<IntT>(x));
}

//! for save as var int
template<class IntT>
inline
typename var_int<IntT>::type
as_var_int(const IntT& x)
{
	return typename var_int<IntT>::type(x);
}

inline unsigned int sizeof_int_aux(char x, boost::mpl::false_) { return 1; }
inline unsigned int sizeof_int_aux(signed char x, boost::mpl::false_) { return 1; }
inline unsigned int sizeof_int_aux(unsigned char x, boost::mpl::false_) { return 1; }

inline unsigned int sizeof_int_aux(uint16_t x, boost::mpl::false_) { return 2; }
inline unsigned int sizeof_int_aux(uint32_t x, boost::mpl::false_) { return 4; }
inline unsigned int sizeof_int_aux(uint64_t x, boost::mpl::false_) { return 8; }

inline unsigned int sizeof_int_aux(int16_t x, boost::mpl::false_) { return 2; }
inline unsigned int sizeof_int_aux(int32_t x, boost::mpl::false_) { return 4; }
inline unsigned int sizeof_int_aux(int64_t x, boost::mpl::false_) { return 8; }

inline unsigned int sizeof_int_aux(var_uint16_t x, boost::mpl::true_)
{
	if (x.t < 1<< 7) return 1;
	if (x.t < 1<<14) return 2;
	return 3;
}

inline unsigned int sizeof_int_aux(var_uint32_t x, boost::mpl::true_)
{
	if (x.t < uint32_t(1)<< 7) return 1;
	if (x.t < uint32_t(1)<<14) return 2;
	if (x.t < uint32_t(1)<<21) return 3;
	if (x.t < uint32_t(1)<<28) return 4;
	return 5;
}

inline unsigned int sizeof_int_aux(var_uint64_t x, boost::mpl::true_)
{
	if (x.t < uint64_t(1)<< 7) return 1;
	if (x.t < uint64_t(1)<<14) return 2;
	if (x.t < uint64_t(1)<<21) return 3;
	if (x.t < uint64_t(1)<<28) return 4;
	if (x.t < uint64_t(1)<<35) return 5;
	if (x.t < uint64_t(1)<<42) return 6;
	if (x.t < uint64_t(1)<<49) return 7;
	if (x.t < uint64_t(1)<<56) return 8;
	return 9;
}

inline unsigned int sizeof_int_aux(var_int16_t x, boost::mpl::true_)
{
	if (x.t >= 0)
		return sizeof_int_aux(var_uint16_t(x.t << 1), boost::mpl::true_());
	else
		return sizeof_int_aux(var_uint16_t(-x.t << 1), boost::mpl::true_());
}

inline unsigned int sizeof_int_aux(var_int32_t x, boost::mpl::true_)
{
	if (x.t >= 0)
		return sizeof_int_aux(var_uint32_t(x.t << 1), boost::mpl::true_());
	else
		return sizeof_int_aux(var_uint32_t(-x.t << 1), boost::mpl::true_());
}

inline unsigned int sizeof_int_aux(var_int64_t x, boost::mpl::true_)
{
	if (x.t >= 0)
		return sizeof_int_aux(var_uint64_t(x.t << 1), boost::mpl::true_());
	else
		return sizeof_int_aux(var_uint64_t(-x.t << 1), boost::mpl::true_());
}

template<class IntType>
unsigned int sizeof_int(IntType x)
{
	return sizeof_int_aux(x, typename is_var_int<IntType>::type());
}

// lowest bit is sign bit
// 
template<class IntType, class UIntType>
inline UIntType var_int_s2u(IntType x)
{
//	BOOST_STATIC_ASSERT(IntType(-1) < IntType(0));
//	BOOST_STATIC_ASSERT(UIntType(-1) > UIntType(0));
	BOOST_STATIC_ASSERT(sizeof(IntType)==sizeof(UIntType));

	if (x < 0) {
		if (std::numeric_limits<IntType>::min() == x)
			return UIntType(1);
		else
			return UIntType(-x << 1 | 1);
	} else
		return UIntType(x << 1);
}

template<class UIntType, class IntType>
inline IntType var_int_u2s(UIntType u)
{
//	BOOST_STATIC_ASSERT(IntType(-1) < IntType(0));
//	BOOST_STATIC_ASSERT(UIntType(-1) > UIntType(0));
	BOOST_STATIC_ASSERT(sizeof(IntType)==sizeof(UIntType));

	if (u & 1) {
		if (0 == u >> 1)
			return std::numeric_limits<IntType>::min();
		else
			return -(IntType)(u >> 1);
	} else
		return (IntType)(u >> 1);
}

inline uint16_t var_int16_s2u(int16_t x) { return var_int_s2u<int16_t, uint16_t>(x); }
inline uint32_t var_int32_s2u(int32_t x) { return var_int_s2u<int32_t, uint32_t>(x); }

inline int16_t var_int16_u2s(uint16_t u) { return var_int_u2s<uint16_t, int16_t>(u); }
inline int32_t var_int32_u2s(uint32_t u) { return var_int_u2s<uint32_t, int32_t>(u); }

#if !defined(BOOST_NO_INT64_T)
inline uint64_t var_int64_s2u(int64_t x) { return var_int_s2u<int64_t, uint64_t>(x); }
inline int64_t var_int64_u2s(uint64_t u) { return var_int_u2s<uint64_t, int64_t>(u); }
#endif

////////////////////////////////////////////////////////////////////////////////////////
FEBIRD_DLL_EXPORT std::pair<uint16_t, int> load_var_uint16(const unsigned char* buf);
FEBIRD_DLL_EXPORT std::pair<uint32_t, int> load_var_uint32(const unsigned char* buf);
FEBIRD_DLL_EXPORT std::pair<uint64_t, int> load_var_uint64(const unsigned char* buf);
FEBIRD_DLL_EXPORT std::pair<int16_t, int> load_var_int16(const unsigned char* buf);
FEBIRD_DLL_EXPORT std::pair<int32_t, int> load_var_int32(const unsigned char* buf);
FEBIRD_DLL_EXPORT std::pair<int64_t, int> load_var_int64(const unsigned char* buf);

//--------------------------------------------------------------------------------------
FEBIRD_DLL_EXPORT unsigned char* save_var_uint16(unsigned char* buf, uint16_t x);
FEBIRD_DLL_EXPORT unsigned char* save_var_uint32(unsigned char* buf, uint32_t x);
FEBIRD_DLL_EXPORT unsigned char* save_var_uint64(unsigned char* buf, uint64_t x);
FEBIRD_DLL_EXPORT unsigned char* save_var_int16(unsigned char* buf, int16_t x);
FEBIRD_DLL_EXPORT unsigned char* save_var_int32(unsigned char* buf, int32_t x);
FEBIRD_DLL_EXPORT unsigned char* save_var_int64(unsigned char* buf, int64_t x);

////////////////////////////////////////////////////////////////////////////////////////
FEBIRD_DLL_EXPORT uint32_t reverse_get_var_uint32(const unsigned char* buf, unsigned char const ** cur);
FEBIRD_DLL_EXPORT int32_t reverse_get_var_int32(const unsigned char* buf, unsigned char const ** cur);

FEBIRD_DLL_EXPORT uint16_t reverse_get_var_uint16(const unsigned char* buf, unsigned char const ** cur);
FEBIRD_DLL_EXPORT int16_t reverse_get_var_int16(const unsigned char* buf, unsigned char const ** cur);

#if !defined(BOOST_NO_INT64_T)
FEBIRD_DLL_EXPORT uint64_t reverse_get_var_uint64(const unsigned char* buf, unsigned char const ** cur);
FEBIRD_DLL_EXPORT int64_t reverse_get_var_int64(const unsigned char* buf, unsigned char const ** cur);
#endif

//#undef FEBIRD_DEFINE_VAR_INT

} // namespace febird


#include "boost/type_traits/detail/bool_trait_undef.hpp"


#endif // __febird_io_var_int_h__


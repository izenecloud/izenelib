/* vim: set tabstop=4 : */
#ifndef __febird_io_byte_swap_h__
#define __febird_io_byte_swap_h__

#include "byte_swap_impl.h"
#include <boost/mpl/bool.hpp>
//#include <boost/type_traits/detail/bool_trait_def.hpp>

namespace febird { 

// inline void byte_swap_in(float&) {}
// inline void byte_swap_in(double&) {}
// inline void byte_swap_in(long double&) {}
// inline void byte_swap_in(char&) {}
// inline void byte_swap_in(signed char&) {}
// inline void byte_swap_in(unsigned char&) {}

inline void byte_swap_in(unsigned short& x, boost::mpl::true_)
{
	x = byte_swap(x);
}
inline void byte_swap_in(short& x, boost::mpl::true_)
{
	x = byte_swap(x);
}

inline void byte_swap_in(unsigned int& i, boost::mpl::true_)
{
	i = byte_swap(i);
}
inline void byte_swap_in(int& i, boost::mpl::true_)
{
	i = byte_swap((unsigned int)i);
}

#if defined(BOOST_HAS_LONG_LONG)
inline void byte_swap_in(unsigned long long& i, boost::mpl::true_)
{
	i = byte_swap(i);
}
inline void byte_swap_in(long long& i, boost::mpl::true_)
{
	i = byte_swap((unsigned long long)(i));
}
#elif defined(BOOST_HAS_MS_INT64)
inline void byte_swap_in(unsigned __int64& i, boost::mpl::true_)
{
	i = byte_swap(i);
}
inline void byte_swap_in(__int64& i, boost::mpl::true_)
{
	i = byte_swap((unsigned __int64)(i));
}
#endif

#if ULONG_MAX == 0xffffffff
inline void byte_swap_in(unsigned long& x, boost::mpl::true_)
{
	x = byte_swap((unsigned int)x);
}
inline void byte_swap_in(long& x, boost::mpl::true_)
{
	x = byte_swap((unsigned int)x);
}
#else
inline void byte_swap_in(unsigned long& x, boost::mpl::true_)
{
	x = byte_swap((unsigned long long)x);
}
inline void byte_swap_in(long& x, boost::mpl::true_) 
{
	x = byte_swap((unsigned long long)x);
}
#endif // ULONG_MAX

template<class T, int Dim>
void byte_swap_in(T (&a)[Dim], boost::mpl::true_)
{
	for (int i = 0; i < Dim; ++i)
		byte_swap_in(a[i], boost::mpl::true_());
}

template<class T>
void byte_swap_in(T&, boost::mpl::false_)
{
	// do nothing
}

//////////////////////////////////////////////////////////////////////////
// DataIO_need_bswap
#if 0

  #define DATA_IO_NEED_BYTE_SWAP(T, cbool) BOOST_TT_AUX_BOOL_TRAIT_SPEC1(DataIO_need_bswap, T, cbool)

  BOOST_TT_AUX_BOOL_TRAIT_DEF1(DataIO_need_bswap, T, true)

#else

  #define DATA_IO_NEED_BYTE_SWAP(T, cbool) \
	template<> struct DataIO_need_bswap<T> : public boost::mpl::bool_<cbool> {};

  template<class T>
  struct DataIO_need_bswap : public boost::mpl::true_ { };

#endif

DATA_IO_NEED_BYTE_SWAP(		    char, false)
DATA_IO_NEED_BYTE_SWAP(  signed char, false)
DATA_IO_NEED_BYTE_SWAP(unsigned char, false)
DATA_IO_NEED_BYTE_SWAP(		   float, false)
DATA_IO_NEED_BYTE_SWAP(		  double, false)
DATA_IO_NEED_BYTE_SWAP(  long double, false)

template<class T1, class T2>
struct DataIO_need_bswap<std::pair<T1, T2> > : public
	boost::mpl::bool_<DataIO_need_bswap<T1>::value||DataIO_need_bswap<T1>::value>
{
};

template<class T1, class T2>
void byte_swap_in(std::pair<T1, T2>& x, typename DataIO_need_bswap<std::pair<T1,T2> >::type)
{
	byte_swap_in(x.first,  typename DataIO_need_bswap<T1>::type());
	byte_swap_in(x.second, typename DataIO_need_bswap<T2>::type());
}

// 这个做成模板只是为了在 DATA_IO_LOAD_SAVE 中
// 生成的 Class::byte_swap_in 是模板的，在没有使用的时候，不实例化
template<class Bswap>
class ByteSwapChain;

// only boost::mpl::true_ is applied
template<> class ByteSwapChain<boost::mpl::true_>
{
// 	template<class T> void do_bswap(T& x, boost::mpl::true_ ) const { byte_swap_in(x); }
// 
// 	template<class T> void do_bswap(T& x, boost::mpl::false_) const { }

public:
	template<class T> ByteSwapChain operator&(T& x) const
	{
	//! 如果这个函数编译失败，很可能是用户代码声明了 DataIO_IsDump_TypeTrue1(T)
	//! 但 DataIO_is_realdump 推导的结果却是不可 dump
	//  用户定义为 DATA_IO_DUMP_RAW_MEM 的类可以被传入，此时 DataIO_need_bswap::value == false
	//	do_bswap(x, typename DataIO_need_bswap<T>::type());
	//	BOOST_STATIC_ASSERT(DataIO_need_bswap<T>::value);
	//	byte_swap_in(x, boost::mpl::true_());
		byte_swap_in(x, typename DataIO_need_bswap<T>::type());
		return *this;
	}
};

template<class T>
void byte_swap(T* p, size_t n)
{ 
	for (size_t i = n; i; --i, ++p)
		byte_swap_in(*p, boost::mpl::true_());
}

template<class Iter>
void byte_swap(Iter first, Iter last)
{ 
	for (; first != last; ++first)
		byte_swap_in(*first, boost::mpl::true_());
}

inline void byte_swap(char* buffer, size_t length) { }
inline void byte_swap(signed char* buffer, size_t length) { }
inline void byte_swap(unsigned char* buffer, size_t length) { }

} // namespace febird

//#include <boost/type_traits/detail/bool_trait_undef.hpp>

#endif // __febird_io_byte_swap_h__



#ifndef IZENE_TYPE_TRAITS_H_
#define IZENE_TYPE_TRAITS_H_

#include <types.h>
#include <vector>
#include <map>
#include <string>
#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>

using namespace boost;


NS_IZENELIB_UTIL_BEGIN


template <typename T>
struct IsMemcpySerial{
	enum {yes = is_arithmetic<T >::value 
		|| is_empty<T>::value || is_array<T>::value,
		no= !yes};
};

template <typename T>
struct IsMemcpySerial<std::vector<T >  >{
	enum {yes =IsMemcpySerial<T>::yes,		
		no= !yes};	
};

/*

template < typename T1, typename T2   >
struct IsMemcpySerial<std::pair<T1, T2>  >
{
	enum {yes = IsMemcpySerial<T1>::yes && IsMemcpySerial<T2>::yes,
		no= !yes};
};



template < typename T1, typename T2  >
struct IsMemcpySerial<boost::tuple<T1, T2>  >
{
	enum {yes = IsMemcpySerial<T1>::yes && IsMemcpySerial<T2>::yes,
		no= !yes};
};



template < typename T1, typename T2, typename T3  >
struct IsMemcpySerial<boost::tuple<T1, T2, T3>  >
{
	enum {yes = IsMemcpySerial<T1>::yes && IsMemcpySerial<T2>::yes && IsMemcpySerial<T3>::yes,
		no= !yes};
};

*/

template <typename T>
struct IsFebirdSerial{
	enum {yes = 0, 
		no = !yes};	
};

NS_IZENELIB_UTIL_END


#define MAKE_FEBIRD_SERIALIZATION(type) \
	 namespace izenelib{namespace util{ \
	 template <>struct IsFebirdSerial<type >{ \
		enum { yes=1, no=!yes}; \
		}; \
		} \
		} 



#define MAKE_MEMCPY_SERIALIZATION(type) \
	namespace izenelib{namespace util{ \
	template <>struct IsMemcpySerial<type >{ \
		enum { yes=1, no=!yes}; \
		}; \
		} \
		}

MAKE_MEMCPY_SERIALIZATION(std::string)
//MAKE_FEBIRD_SERIALIZATION(std::string)

#endif /*IZENE_TYPE_TRAITS_H_*/

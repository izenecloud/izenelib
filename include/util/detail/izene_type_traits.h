#ifndef IZENE_TYPE_TRAITS_H_
#define IZENE_TYPE_TRAITS_H_

#include <types.h>

#include <vector>
#include <string>
#include <boost/type_traits.hpp>

using namespace boost;

NS_IZENELIB_UTIL_BEGIN

template <typename T>
struct IsMemcpySerial{
	enum {yes = is_arithmetic<T>::value 
		|| is_empty<T>::value,
		no= !yes};
};

template <typename T>
struct IsMemcpySerial<std::vector<T> >{
	enum {yes = is_arithmetic<T>::value 
		|| is_empty<T>::value,		
		no= !yes};	
};


template <typename T>
struct IsFebirdSerial{
	enum {yes = 0, 
		no= !yes};	
};


#define MAKE_FEBIRD_SERIALIZATION(type) 	\
namespace izenelib{namespace util{	 	\
	template <>struct IsFebirdSerial<type>{	 \
		enum { yes = 1, no= 0}; 		\	
	}; \
	}  \
	}


#define MAKE_MEMCPY_SERIALIZATION(type) 	\
	namespace izenelib{namespace util{	\
	template <>struct IsMemcpySerial<type>{ \
		enum {yes = 1, no= 0};  \		
	}; \
	}  \
	}

NS_IZENELIB_UTIL_END

MAKE_MEMCPY_SERIALIZATION(std::string)
		
#endif /*IZENE_TYPE_TRAITS_H_*/

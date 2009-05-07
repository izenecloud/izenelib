#ifndef COPY_H_
#define COPY_H_

//
// opt::copy
// same semantics as std::copy
// calls memcpy where appropriate.
//

#include <boost/type_traits.hpp>
NS_IZENELIB_UTIL_BEGIN
{

template<typename I1, typename I2, bool b>
I2 copy_imp(I1 first, I1 last, I2 out, const boost::integral_constant<bool, b>&)
{
   while(first != last)
   {
      *out = *first;
      ++out;
      ++first;
   }
   return out;
}

template<typename T>
T* copy_imp(const T* first, const T* last, T* out, const boost::true_type&)
{
   memcpy(out, first, (last-first)*sizeof(T));
   return out+(last-first);
}


}

template<typename I1, typename I2>
inline I2 copy(I1 first, I1 last, I2 out)
{
   //
   // We can copy with memcpy if T has a trivial assignment operator,
   // and if the iterator arguments are actually pointers (this last
   // requirement we detect with overload resolution):
   //
   typedef typename std::iterator_traits<I1>::value_type value_type;
   return detail::copy_imp(first, last, out, boost::has_trivial_assign<value_type>());
}

NS_IZENELIB_UTIL_END

#endif /*COPY_H_*/

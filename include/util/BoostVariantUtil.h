#ifndef BOOST_VARIANT_UTIL_H
#define BOOST_VARIANT_UTIL_H

#include <types.h>

#include <boost/variant.hpp>

NS_IZENELIB_UTIL_BEGIN

template<class TFunctor, class TVariant>
void boost_variant_visit(TFunctor functor_, TVariant &variant)
{
   boost::apply_visitor(functor_,variant);
}

template<class TFunctor, class TVariant1, class TVariant2>
void boost_variant_visit(TFunctor functor_, TVariant1 &variant1, TVariant2 &variant2)
{
   boost::apply_visitor(functor_,variant1,variant2);
}

NS_IZENELIB_UTIL_END

#endif


#ifndef IZENE_TYPE_TRAITS_H_
#define IZENE_TYPE_TRAITS_H_

#include <types.h>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <string>
#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>

NS_IZENELIB_UTIL_BEGIN

template <typename T>
struct IsFixedType
{
    enum
    {
        yes = boost::is_arithmetic<T>::value || boost::is_empty<T>::value,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsFixedType<std::pair<T1, T2> >
{
    enum
    {
        yes = IsFixedType<T1>::yes && IsFixedType<T2>::yes,
        no = !yes
    };
};

template <>
struct IsFixedType<int128_t>
{
    enum
    {
        yes = 1,
        no = !yes
    };
};

template <>
struct IsFixedType<uint128_t>
{
    enum
    {
        yes = 1,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsFixed
{
    enum
    {
        yes = IsFixedType<T1>::yes && IsFixedType<T2>::yes ,
        no = !yes
    };
};

template <typename T>
struct IsMemcpySerial
{
    enum
    {
        yes = IsFixedType<T>::yes,
        no = !yes
    };
};

template <typename T>
struct IsMemcpySerial<std::vector<std::pair<T, T> > >
{
    enum
    {
        yes = IsFixedType<T>::yes,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsMemcpySerial<std::vector<std::pair<T1, T2> > >
{
    enum
    {
        yes = 0,
        no = !yes
    };
};

template <typename T>
struct IsMemcpySerial<std::vector<T> >
{
    enum
    {
        yes = IsFixedType<T>::yes,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsMemcpySerial<std::pair<T1, T2> >
{
    enum
    {
        yes = IsFixedType<T1>::yes && IsFixedType<T2>::yes,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsMemcpySerial<boost::tuple<T1, T2> >
{
    enum
    {
        yes = IsFixedType<T1>::yes && IsFixedType<T2>::yes,
        no = !yes
    };
};

template <typename T1, typename T2, typename T3>
struct IsMemcpySerial<boost::tuple<T1, T2, T3> >
{
    enum
    {
        yes = IsFixedType<T1>::yes
            && IsFixedType<T2>::yes
            && IsFixedType<T3>::yes,
        no = !yes
    };
};

template <typename T>
struct IsFebirdSerial
{
    enum
    {
        yes = 0,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsFebirdSerial<std::pair<T1, T2> >
{
    enum
    {
        yes = IsFebirdSerial<T1>::yes && IsFebirdSerial<T2>::yes,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsFebirdSerial<boost::tuple<T1, T2> >
{
    enum
    {
        yes = IsFebirdSerial<T1>::yes && IsFebirdSerial<T2>::yes,
        no = !yes
    };
};

template <typename T1, typename T2, typename T3>
struct IsFebirdSerial<boost::tuple<T1, T2, T3> >
{
    enum
    {
        yes = IsFebirdSerial<T1>::yes && IsFebirdSerial<T2>::yes
                && IsFebirdSerial<T3>::yes,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsFebirdSerial<std::vector<std::pair<T1, T2> > >
{
    enum
    {
        yes = 1,
        no = !yes
    };
};

template <typename T>
struct IsFebirdSerial<std::vector<T> >
{
    enum
    {
        yes = IsFebirdSerial<T>::yes,
        no = !yes
    };
};

template <typename T>
struct IsFebirdSerial<std::list<T> >
{
    enum
    {
        yes = IsFebirdSerial<T>::yes,
        no = !yes
    };
};

template <typename T>
struct IsFebirdSerial<std::set<T> >
{
    enum
    {
        yes = IsFebirdSerial<T>::yes,
        no = !yes
    };
};

template <typename T1, typename T2>
struct IsFebirdSerial<std::map<T1, T2> >
{
    enum
    {
        yes = IsFebirdSerial<T1>::yes && IsFebirdSerial<T2>::yes,
        no = !yes
    };
};

NS_IZENELIB_UTIL_END

#define MAKE_FEBIRD_SERIALIZATION(...) \
namespace izenelib \
{ \
namespace util \
{ \
    template <>struct IsFebirdSerial< __VA_ARGS__ > \
    { \
        enum { yes = 1, no = !yes }; \
    }; \
} \
}

#define MAKE_MEMCPY_SERIALIZATION(...) \
namespace izenelib \
{ \
namespace util \
{ \
    template <>struct IsMemcpySerial< __VA_ARGS__ > \
    { \
        enum { yes = 1, no = !yes }; \
    }; \
} \
}

MAKE_MEMCPY_SERIALIZATION(std::string)
MAKE_FEBIRD_SERIALIZATION(std::vector<std::string>)

//////////////////////////////////////////////////////

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/preprocessor/repetition.hpp>

//serialization for boost tuple
namespace boost { namespace serialization {

#define GENERATE_ELEMENT_SERIALIZE(z, which, unused) \
    ar & boost::serialization::make_nvp("element", t.get< which >());

#define GENERATE_TUPLE_SERIALIZE(z, nargs, unused) \
template <typename Archive, BOOST_PP_ENUM_PARAMS(nargs, typename T) > \
void serialize(Archive & ar, \
        boost::tuple< BOOST_PP_ENUM_PARAMS(nargs,T) > & t, \
        const unsigned int version) \
{ \
BOOST_PP_REPEAT_FROM_TO(0, nargs, GENERATE_ELEMENT_SERIALIZE, ~) \
}

BOOST_PP_REPEAT_FROM_TO(1, 10, GENERATE_TUPLE_SERIALIZE, ~);

}}

#endif /*IZENE_TYPE_TRAITS_H_*/
